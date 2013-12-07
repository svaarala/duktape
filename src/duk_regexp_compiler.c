/*
 *  Regexp compilation.
 *
 *  See doc/regexp.txt for a discussion of the compilation approach and
 *  current limitations.
 */

#include "duk_internal.h"

#ifdef DUK_USE_REGEXP_SUPPORT

/*
 *  Helper macros
 */

#ifdef DUK_BUFLEN
#undef DUK_BUFLEN
#endif

#define DUK_BUFLEN(re_ctx)   DUK_HBUFFER_GET_SIZE((duk_hbuffer *) re_ctx->buf)

/*
 *  Encoding helpers
 */

static duk_uint32_t encode_i32(duk_int32_t x) {
	if (x < 0) {
		return ((duk_uint32_t) (-x)) * 2 + 1;
	} else {
		return ((duk_uint32_t) x) * 2;
	}
}

static duk_uint32_t insert_u32(duk_re_compiler_ctx *re_ctx, duk_uint32_t offset, duk_uint32_t x) {
	return duk_hbuffer_insert_xutf8(re_ctx->thr, re_ctx->buf, offset, x);
}

static duk_uint32_t append_u32(duk_re_compiler_ctx *re_ctx, duk_uint32_t x) {
	return duk_hbuffer_append_xutf8(re_ctx->thr, re_ctx->buf, x);
}

static duk_uint32_t insert_i32(duk_re_compiler_ctx *re_ctx, duk_uint32_t offset, duk_int32_t x) {
	return duk_hbuffer_insert_xutf8(re_ctx->thr, re_ctx->buf, offset, encode_i32(x));
}

#if 0  /* unused */
static duk_uint32_t append_i32(duk_re_compiler_ctx *re_ctx, duk_int32_t x) {
	return duk_hbuffer_append_xutf8(re_ctx->thr, re_ctx->buf, encode_i32(x));
}
#endif

/* special helper for emitting u16 lists (used for character ranges for built-in char classes) */
static duk_uint32_t append_u16_list(duk_re_compiler_ctx *re_ctx, duk_uint16_t *values, duk_uint32_t count) {
	duk_uint32_t len = 0;
	while (count > 0) {
		len += append_u32(re_ctx, (duk_uint32_t) (*values++));
		count--;
	}
	return len;
}

static void insert_slice(duk_re_compiler_ctx *re_ctx, duk_uint32_t offset, duk_uint32_t data_offset, duk_uint32_t data_length) {
	duk_hbuffer_insert_slice(re_ctx->thr, re_ctx->buf, offset, data_offset, data_length);
}

static void append_slice(duk_re_compiler_ctx *re_ctx, duk_uint32_t data_offset, duk_uint32_t data_length) {
	duk_hbuffer_append_slice(re_ctx->thr, re_ctx->buf, data_offset, data_length);
}

static void remove_slice(duk_re_compiler_ctx *re_ctx, duk_uint32_t offset, duk_uint32_t length) {
	duk_hbuffer_remove_slice(re_ctx->thr, re_ctx->buf, offset, length);
}

/*
 *  Insert a jump offset at 'offset' to complete an instruction
 *  (the jump offset is always the last component of an instruction).
 *  The 'skip' argument must be computed relative to 'offset',
 *  -without- taking into account the skip field being inserted.
 *
 *       ... A B C ins X Y Z ...   (ins may be a JUMP, SPLIT1/SPLIT2, etc)
 *   =>  ... A B C ins SKIP X Y Z
 */

static duk_uint32_t insert_jump_offset(duk_re_compiler_ctx *re_ctx, duk_uint32_t offset, duk_int32_t skip) {
	duk_uint32_t len;

	/*
	 *  Computing the final (adjusted) skip value, which is relative
	 *  to the first byte of the next instruction, is a bit tricky
	 *  because of the variable length UTF-8 encoding.
	 *
	 *  See doc/regexp.txt for discussion.
	 */

	/* FIXME: solve into closed form (smaller code) */

	if (skip < 0) {
		/* two encoding attempts suffices */
		len = duk_unicode_get_xutf8_length((duk_codepoint_t) encode_i32(skip));
		len = duk_unicode_get_xutf8_length((duk_codepoint_t) encode_i32(skip - len));
		DUK_ASSERT(duk_unicode_get_xutf8_length(encode_i32(skip - len)) == len);  /* no change */
		skip -= len;
	}
	return insert_i32(re_ctx, offset, skip);
}

static duk_uint32_t append_jump_offset(duk_re_compiler_ctx *re_ctx, duk_int32_t skip) {
	return insert_jump_offset(re_ctx, DUK_BUFLEN(re_ctx), skip);
}

/*
 *  duk_re_range_callback for generating character class ranges.
 *
 *  When ignoreCase is false, the range is simply emitted as is.
 *  We don't, for instance, eliminate duplicates or overlapping
 *  ranges in a character class.
 *
 *  When ignoreCase is true, the range needs to be normalized through
 *  canonicalization.  Unfortunately a canonicalized version of a
 *  continuous range is not necessarily continuous (e.g. [x-{] is
 *  continuous but [X-{] is not).  The current algorithm creates the
 *  canonicalized range(s) space efficiently at the cost of compile
 *  time execution time (see doc/regexp.txt for discussion).
 *
 *  Note that the ctx->nranges is a context-wide temporary value
 *  (this is OK because there cannot be multiple character classes
 *  being parsed simultaneously).
 */

static void generate_ranges(void *userdata, duk_uint32_t r1, duk_uint32_t r2, int direct) {
	duk_re_compiler_ctx *re_ctx = (duk_re_compiler_ctx *) userdata;

	DUK_DDPRINT("generate_ranges(): re_ctx=%p, range=[%d,%d] direct=%d", re_ctx, r1, r2, direct);

	if (!direct && (re_ctx->re_flags & DUK_RE_FLAG_IGNORE_CASE)) {
		/*
		 *  Canonicalize a range, generating result ranges as necessary.
		 *  Needs to exhaustively scan the entire range (at most 65536
		 *  code points).  If 'direct' is set, caller (lexer) has ensured
		 *  that the range is already canonicalization compatible (this
		 *  is used to avoid unnecessary canonicalization of built-in
		 *  ranges like \W, which are not affected by canonicalization).
		 *
		 *  NOTE: here is one place where we don't want to support chars
		 *  outside the BMP, because the exhaustive search would be
		 *  massively larger.
		 */

		duk_uint32_t i;
		duk_uint32_t t;
		duk_uint32_t r_start, r_end;

		r_start = duk_unicode_re_canonicalize_char(re_ctx->thr, r1);
		r_end = r_start;
		for (i = r1 + 1; i <= r2; i++) {
			t = duk_unicode_re_canonicalize_char(re_ctx->thr, i);
			if (t == r_end + 1) {
				r_end = t;
			} else {
				DUK_DDPRINT("canonicalized, emit range: [%d,%d]", r_start, r_end);
				append_u32(re_ctx, r_start);
				append_u32(re_ctx, r_end);
				re_ctx->nranges++;
				r_start = t;
				r_end = t;
			}
		}
		DUK_DDPRINT("canonicalized, emit range: [%d,%d]", r_start, r_end);
		append_u32(re_ctx, r_start);
		append_u32(re_ctx, r_end);
		re_ctx->nranges++;
	} else {
		DUK_DDPRINT("direct, emit range: [%d,%d]", r1, r2);
		append_u32(re_ctx, r1);
		append_u32(re_ctx, r2);
		re_ctx->nranges++;
	}
}

/*
 *  Parse regexp Disjunction.  Most of regexp compilation happens here.
 *
 *  Handles Disjunction, Alternative, and Term productions directly without
 *  recursion.  The only constructs requiring recursion are positive/negative
 *  lookaheads, capturing parentheses, and non-capturing parentheses.
 *
 *  The function determines whether the entire disjunction is a 'simple atom'
 *  (see doc/regexp.txt discussion on 'simple quantifiers') and if so,
 *  returns the atom character length which is needed by the caller to keep
 *  track of its own atom character length.  A disjunction with more than one
 *  alternative is never considered a simple atom (although in some cases
 *  that might be the case).
 *
 *  Return value: simple atom character length or < 0 if not a simple atom.
 *  Appends the bytecode for the disjunction matcher to the end of the temp
 *  buffer.
 *
 *  Regexp top level structure is:
 *
 *    Disjunction = Term*
 *                | Term* | Disjunction
 *
 *    Term = Assertion
 *         | Atom
 *         | Atom Quantifier
 *
 *  An empty Term sequence is a valid disjunction alternative (e.g. /|||c||/).
 *
 *  Notes:
 *
 *    * Tracking of the 'simple-ness' of the current atom vs. the entire
 *      disjunction are separate matters.  For instance, the disjunction
 *      may be complex, but individual atoms may be simple.  Furthermore,
 *      simple quantifiers are used whenever possible, even if the
 *      disjunction as a whole is complex.
 *
 *    * The estimate of whether an atom is simple is conservative now,
 *      and it would be possible to expand it.  For instance, captures
 *      cause the disjunction to be marked complex, even though captures
 *      -can- be handled by simple quantifiers with some minor modifications.
 *
 *    * Disjunction 'tainting' as 'complex' is handled at the end of the
 *      main for loop collectively for atoms.  Assertions, quantifiers,
 *      and '|' tokens need to taint the result manually if necessary.
 *      Assertions cannot add to result char length, only atoms (and
 *      quantifiers) can; currently quantifiers will taint the result
 *      as complex though.
 */

static duk_int32_t parse_disjunction(duk_re_compiler_ctx *re_ctx, int expect_eof) {
	duk_int32_t atom_start_offset = -1;
	duk_int32_t atom_char_length = 0;   /* negative -> complex atom */
	duk_int32_t unpatched_disjunction_split = -1;
	duk_int32_t unpatched_disjunction_jump = -1;
	duk_uint32_t entry_offset = DUK_BUFLEN(re_ctx);
	duk_int32_t res = 0;	/* -1 if disjunction is complex, char length if simple */

	if (re_ctx->recursion_depth >= re_ctx->recursion_limit) {
		DUK_ERROR(re_ctx->thr, DUK_ERR_INTERNAL_ERROR,
		          "regexp compiler recursion limit reached");
	}
	re_ctx->recursion_depth++;

	for (;;) {
		duk_int32_t new_atom_char_length;   /* char length of the atom parsed in this loop */
		duk_int32_t new_atom_start_offset;  /* bytecode start offset of the atom parsed in this loop
		                                 * (allows quantifiers to copy the atom bytecode)
		                                 */

		duk_lexer_parse_re_token(&re_ctx->lex, &re_ctx->curr_token);

		DUK_DDPRINT("re token: %d (num=%d, char=%c)",
		            re_ctx->curr_token.t,
		            re_ctx->curr_token.num,
		            (re_ctx->curr_token.num >= 0x20 && re_ctx->curr_token.num <= 0x7e) ?
		            (char) re_ctx->curr_token.num : '?');

		/* set by atom case clauses */
		new_atom_start_offset = -1;
		new_atom_char_length = -1;

		switch (re_ctx->curr_token.t) {
		case DUK_RETOK_DISJUNCTION: {
			/*
			 *  The handling here is a bit tricky.  If a previous '|' has been processed,
			 *  we have a pending split1 and a pending jump (for a previous match).  These
			 *  need to be back-patched carefully.  See docs for a detailed example.
			 */

			/* patch pending jump and split */
			if (unpatched_disjunction_jump >= 0) {
				duk_uint32_t offset;

				DUK_ASSERT(unpatched_disjunction_split >= 0);
				offset = unpatched_disjunction_jump;
				offset += insert_jump_offset(re_ctx,
				                             offset,
				                             DUK_BUFLEN(re_ctx) - offset);
				/* offset is now target of the pending split (right after jump) */
				insert_jump_offset(re_ctx,
				                   unpatched_disjunction_split,
				                   offset - unpatched_disjunction_split);
			}

			/* add a new pending split to the beginning of the entire disjunction */
			(void) insert_u32(re_ctx,
			                  entry_offset,
			                  DUK_REOP_SPLIT1);   /* prefer direct execution */
			unpatched_disjunction_split = entry_offset + 1;   /* +1 for opcode */

			/* add a new pending match jump for latest finished alternative */
			append_u32(re_ctx, DUK_REOP_JUMP);
			unpatched_disjunction_jump = DUK_BUFLEN(re_ctx);

			/* 'taint' result as complex */
			res = -1;
			break;
		}
		case DUK_RETOK_QUANTIFIER: {
			if (atom_start_offset < 0) {
				DUK_ERROR(re_ctx->thr, DUK_ERR_SYNTAX_ERROR,
				          "quantifier without preceding atom");
			}
			if (re_ctx->curr_token.qmin > re_ctx->curr_token.qmax) {
				DUK_ERROR(re_ctx->thr, DUK_ERR_SYNTAX_ERROR,
				          "quantifier values invalid (qmin > qmax)");
			}
			if (atom_char_length >= 0) {
				/*
				 *  Simple atom
				 *
				 *  If atom_char_length is zero, we'll have unbounded execution time for e.g.
			 	 *  /()*x/.exec('x').  We can't just skip the match because it might have some
				 *  side effects (for instance, if we allowed captures in simple atoms, the
				 *  capture needs to happen).  The simple solution below is to force the
				 *  quantifier to match at most once, since the additional matches have no effect.
				 */
				int atom_code_length;
				duk_uint32_t offset;
				duk_uint32_t qmin, qmax;

				qmin = re_ctx->curr_token.qmin;
				qmax = re_ctx->curr_token.qmax;
				if (atom_char_length == 0) {
					/* qmin and qmax will be 0 or 1 */
					if (qmin > 1) {
						qmin = 1;
					}
					if (qmax > 1) {
						qmax = 1;
					}
				}

				append_u32(re_ctx, DUK_REOP_MATCH);   /* complete 'sub atom' */
				atom_code_length = DUK_BUFLEN(re_ctx) - atom_start_offset;

				offset = atom_start_offset;
				if (re_ctx->curr_token.greedy) {
					offset += insert_u32(re_ctx, offset, DUK_REOP_SQGREEDY);
					offset += insert_u32(re_ctx, offset, qmin);
					offset += insert_u32(re_ctx, offset, qmax);
					offset += insert_u32(re_ctx, offset, atom_char_length);
					offset += insert_jump_offset(re_ctx, offset, atom_code_length);
				} else {
					offset += insert_u32(re_ctx, offset, DUK_REOP_SQMINIMAL);
					offset += insert_u32(re_ctx, offset, qmin);
					offset += insert_u32(re_ctx, offset, qmax);
					offset += insert_jump_offset(re_ctx, offset, atom_code_length);
				}
			} else {
				/*
				 *  Complex atom
				 *
				 *  The original code is used as a template, and removed at the end
				 *  (this differs from the handling of simple quantifiers).
				 *
				 *  NOTE: there is no current solution for empty atoms in complex
				 *  quantifiers.  This would need some sort of a 'progress' instruction.
				 *
				 *  XXX: impose limit on maximum result size, i.e. atom_code_len * atom_copies?
				 */
				duk_int32_t atom_code_length;
				int atom_copies;
				duk_uint32_t tmp_qmin, tmp_qmax;

				/* pre-check how many atom copies we're willing to make (atom_copies not needed below) */
				atom_copies = (re_ctx->curr_token.qmax == DUK_RE_QUANTIFIER_INFINITE) ?
				              re_ctx->curr_token.qmin : re_ctx->curr_token.qmax;
				if (atom_copies > DUK_RE_MAX_ATOM_COPIES) {
					DUK_ERROR(re_ctx->thr, DUK_ERR_INTERNAL_ERROR,
					          "quantifier expansion requires too many atom copies");
				}

				atom_code_length = DUK_BUFLEN(re_ctx) - atom_start_offset;

				/* insert the required matches (qmin) by copying the atom */
				tmp_qmin = re_ctx->curr_token.qmin;
				tmp_qmax = re_ctx->curr_token.qmax;
				while (tmp_qmin > 0) {
					append_slice(re_ctx, atom_start_offset, atom_code_length);
					tmp_qmin--;
					if (tmp_qmax != DUK_RE_QUANTIFIER_INFINITE) {
						tmp_qmax--;
					}
				}
				DUK_ASSERT(tmp_qmin == 0);

				/* insert code for matching the remainder - infinite or finite */
				if (tmp_qmax == DUK_RE_QUANTIFIER_INFINITE) {
					/* reuse last emitted atom for remaining 'infinite' quantifier */

					if (re_ctx->curr_token.qmin == 0) {
						/* Special case: original qmin was zero so there is nothing
						 * to repeat.  Emit an atom copy but jump over it here.
						 */
						append_u32(re_ctx, DUK_REOP_JUMP);
						append_jump_offset(re_ctx, atom_code_length);
						append_slice(re_ctx, atom_start_offset, atom_code_length);
					}
					if (re_ctx->curr_token.greedy) {
						append_u32(re_ctx, DUK_REOP_SPLIT2);   /* prefer jump */
					} else {
						append_u32(re_ctx, DUK_REOP_SPLIT1);   /* prefer direct */
					}
					append_jump_offset(re_ctx, -atom_code_length - 1);  /* -1 for opcode */
				} else {
					/*
					 *  The remaining matches are emitted as sequence of SPLITs and atom
					 *  copies; the SPLITs skip the remaining copies and match the sequel.
					 *  This sequence needs to be emitted starting from the last copy
					 *  because the SPLITs are variable length due to the variable length
					 *  skip offset.  This causes a lot of memory copying now.
					 *
					 *  Example structure (greedy, match maximum # atoms):
					 *
					 *      SPLIT1 LSEQ
					 *      (atom)
					 *      SPLIT1 LSEQ    ; <- the byte length of this instruction is needed
					 *      (atom)	       ; to encode the above SPLIT1 correctly
					 *      ...
					 *   LSEQ:
					 */
					duk_uint32_t offset = DUK_BUFLEN(re_ctx);
					while (tmp_qmax > 0) {
						insert_slice(re_ctx, offset, atom_start_offset, atom_code_length);
						if (re_ctx->curr_token.greedy) {
							insert_u32(re_ctx, offset, DUK_REOP_SPLIT1);   /* prefer direct */
						} else {
							insert_u32(re_ctx, offset, DUK_REOP_SPLIT2);   /* prefer jump */
						}
						insert_jump_offset(re_ctx,
						                   offset + 1,   /* +1 for opcode */
						                   DUK_BUFLEN(re_ctx) - (offset + 1));
						tmp_qmax--;
					}
				}

				/* remove the original 'template' atom */
				remove_slice(re_ctx, atom_start_offset, atom_code_length);
			}

			/* 'taint' result as complex */
			res = -1;
			break;
		}
		case DUK_RETOK_ASSERT_START: {
			append_u32(re_ctx, DUK_REOP_ASSERT_START);
			break;
		}
		case DUK_RETOK_ASSERT_END: {
			append_u32(re_ctx, DUK_REOP_ASSERT_END);
			break;
		}
		case DUK_RETOK_ASSERT_WORD_BOUNDARY: {
			append_u32(re_ctx, DUK_REOP_ASSERT_WORD_BOUNDARY);
			break;
		}
		case DUK_RETOK_ASSERT_NOT_WORD_BOUNDARY: {
			append_u32(re_ctx, DUK_REOP_ASSERT_NOT_WORD_BOUNDARY);
			break;
		}
		case DUK_RETOK_ASSERT_START_POS_LOOKAHEAD:
		case DUK_RETOK_ASSERT_START_NEG_LOOKAHEAD: {
			duk_uint32_t offset;
			duk_uint32_t opcode = (re_ctx->curr_token.t == DUK_RETOK_ASSERT_START_POS_LOOKAHEAD) ?
			                      DUK_REOP_LOOKPOS : DUK_REOP_LOOKNEG;

			offset = DUK_BUFLEN(re_ctx);
			(void) parse_disjunction(re_ctx, 0);
			append_u32(re_ctx, DUK_REOP_MATCH);

			(void) insert_u32(re_ctx, offset, opcode);
			(void) insert_jump_offset(re_ctx,
			                          offset + 1,   /* +1 for opcode */
			                          DUK_BUFLEN(re_ctx) - (offset + 1));

			/* 'taint' result as complex -- this is conservative,
			 * as lookaheads do not backtrack.
			 */
			res = -1;
			break;
		}
		case DUK_RETOK_ATOM_PERIOD: {
			new_atom_char_length = 1;
			new_atom_start_offset = DUK_BUFLEN(re_ctx);
			append_u32(re_ctx, DUK_REOP_PERIOD);
			break;
		}
		case DUK_RETOK_ATOM_CHAR: {
			/* Note: successive characters could be joined into string matches
			 * but this is not trivial (consider e.g. '/xyz+/); see docs for
			 * more discussion.
			 */
			duk_uint32_t ch;

			new_atom_char_length = 1;
			new_atom_start_offset = DUK_BUFLEN(re_ctx);
			append_u32(re_ctx, DUK_REOP_CHAR);
			ch = re_ctx->curr_token.num;
			if (re_ctx->re_flags & DUK_RE_FLAG_IGNORE_CASE) {
				ch = duk_unicode_re_canonicalize_char(re_ctx->thr, ch);
			}
			append_u32(re_ctx, ch);
			break;
		}
		case DUK_RETOK_ATOM_DIGIT:
		case DUK_RETOK_ATOM_NOT_DIGIT: {
			new_atom_char_length = 1;
			new_atom_start_offset = DUK_BUFLEN(re_ctx);
			append_u32(re_ctx,
			           (re_ctx->curr_token.t == DUK_RETOK_ATOM_DIGIT) ?
			           DUK_REOP_RANGES : DUK_REOP_INVRANGES);
			append_u32(re_ctx, sizeof(duk_unicode_re_ranges_digit) / (2 * sizeof(duk_uint16_t)));
			append_u16_list(re_ctx, duk_unicode_re_ranges_digit, sizeof(duk_unicode_re_ranges_digit) / sizeof(duk_uint16_t));
			break;
		}
		case DUK_RETOK_ATOM_WHITE:
		case DUK_RETOK_ATOM_NOT_WHITE: {
			new_atom_char_length = 1;
			new_atom_start_offset = DUK_BUFLEN(re_ctx);
			append_u32(re_ctx,
			           (re_ctx->curr_token.t == DUK_RETOK_ATOM_WHITE) ?
			           DUK_REOP_RANGES : DUK_REOP_INVRANGES);
			append_u32(re_ctx, sizeof(duk_unicode_re_ranges_white) / (2 * sizeof(duk_uint16_t)));
			append_u16_list(re_ctx, duk_unicode_re_ranges_white, sizeof(duk_unicode_re_ranges_white) / sizeof(duk_uint16_t));
			break;
		}
		case DUK_RETOK_ATOM_WORD_CHAR:
		case DUK_RETOK_ATOM_NOT_WORD_CHAR: {
			new_atom_char_length = 1;
			new_atom_start_offset = DUK_BUFLEN(re_ctx);
			append_u32(re_ctx,
			           (re_ctx->curr_token.t == DUK_RETOK_ATOM_WORD_CHAR) ?
			           DUK_REOP_RANGES : DUK_REOP_INVRANGES);
			append_u32(re_ctx, sizeof(duk_unicode_re_ranges_wordchar) / (2 * sizeof(duk_uint16_t)));
			append_u16_list(re_ctx, duk_unicode_re_ranges_wordchar, sizeof(duk_unicode_re_ranges_wordchar) / sizeof(duk_uint16_t));
			break;
		}
		case DUK_RETOK_ATOM_BACKREFERENCE: {
			duk_uint32_t backref = (duk_uint32_t) re_ctx->curr_token.num;
			if (backref > re_ctx->highest_backref) {
				re_ctx->highest_backref = backref;
			}
			new_atom_char_length = -1;   /* mark as complex */
			new_atom_start_offset = DUK_BUFLEN(re_ctx);
			append_u32(re_ctx, DUK_REOP_BACKREFERENCE);
			append_u32(re_ctx, backref);
			break;
		}
		case DUK_RETOK_ATOM_START_CAPTURE_GROUP: {
			duk_uint32_t cap;

			new_atom_char_length = -1;   /* mark as complex (capture handling) */
			new_atom_start_offset = DUK_BUFLEN(re_ctx);
			cap = ++re_ctx->captures;
			append_u32(re_ctx, DUK_REOP_SAVE);
			append_u32(re_ctx, cap * 2);
			(void) parse_disjunction(re_ctx, 0);  /* retval (sub-atom char length) unused, tainted as complex above */
			append_u32(re_ctx, DUK_REOP_SAVE);
			append_u32(re_ctx, cap * 2 + 1);
			break;
		}
		case DUK_RETOK_ATOM_START_NONCAPTURE_GROUP: {
			new_atom_char_length = parse_disjunction(re_ctx, 0);
			new_atom_start_offset = DUK_BUFLEN(re_ctx);
			break;
		}
		case DUK_RETOK_ATOM_START_CHARCLASS:
		case DUK_RETOK_ATOM_START_CHARCLASS_INVERTED: {
			/*
			 *  Range parsing is done with a special lexer function which calls
			 *  us for every range parsed.  This is different from how rest of
			 *  the parsing works, but avoids a heavy, arbitrary size intermediate
			 *  value type to hold the ranges.
			 *
			 *  Another complication is the handling of character ranges when
			 *  case insensitive matching is used (see docs for discussion).
			 *  The range handler callback given to the lexer takes care of this
			 *  as well.
			 *
			 *  Note that duplicate ranges are not eliminated when parsing character
			 *  classes, so that canonicalization of
			 *
			 *    [0-9a-fA-Fx-{]
			 *
			 *  creates the result (note the duplicate ranges):
			 *
			 *    [0-9A-FA-FX-Z{-{]
			 *
			 *  where [x-{] is split as a result of canonicalization.  The duplicate
			 *  ranges are not a semantics issue: they work correctly.
			 */

			duk_uint32_t offset;

			DUK_DDPRINT("character class");

			/* insert ranges instruction, range count patched in later */
			new_atom_char_length = 1;
			new_atom_start_offset = DUK_BUFLEN(re_ctx);
			append_u32(re_ctx,
			           (re_ctx->curr_token.t == DUK_RETOK_ATOM_START_CHARCLASS) ?
			           DUK_REOP_RANGES : DUK_REOP_INVRANGES);
			offset = DUK_BUFLEN(re_ctx);    /* patch in range count later */

			/* parse ranges until character class ends */
			re_ctx->nranges = 0;    /* note: ctx-wide temporary */
			duk_lexer_parse_re_ranges(&re_ctx->lex, generate_ranges, (void *) re_ctx);

			/* insert range count */
			insert_u32(re_ctx, offset, re_ctx->nranges);
			break;
		}
		case DUK_RETOK_ATOM_END_GROUP: {
			if (expect_eof) {
				DUK_ERROR(re_ctx->thr, DUK_ERR_SYNTAX_ERROR,
				          "unexpected closing parenthesis");
			}
			goto done;
		}
		case DUK_RETOK_EOF: {
			if (!expect_eof) {
				DUK_ERROR(re_ctx->thr, DUK_ERR_SYNTAX_ERROR,
				          "unexpected end of pattern");
			}
			goto done;
		}
		default: {
			DUK_ERROR(re_ctx->thr, DUK_ERR_SYNTAX_ERROR,
			          "unexpected token in regexp");
		}
		}

		/* a complex (new) atom taints the result */
		if (new_atom_start_offset >= 0) {
			if (new_atom_char_length < 0) {
				res = -1;
			} else if (res >= 0) {
				/* only advance if not tainted */
				res += new_atom_char_length;
			}
		}

		/* record previous atom info in case next token is a quantifier */
		atom_start_offset = new_atom_start_offset;
		atom_char_length = new_atom_char_length;
	}

 done:

	/* finish up pending jump and split for last alternative */
	if (unpatched_disjunction_jump >= 0) {
		duk_uint32_t offset;

		DUK_ASSERT(unpatched_disjunction_split >= 0);
		offset = unpatched_disjunction_jump;
		offset += insert_jump_offset(re_ctx,
		                             offset,
		                             DUK_BUFLEN(re_ctx) - offset);
		/* offset is now target of the pending split (right after jump) */
		insert_jump_offset(re_ctx,
		                   unpatched_disjunction_split,
		                   offset - unpatched_disjunction_split);
	}

	re_ctx->recursion_depth--;

	return res;
}

/*
 *  Flags parsing (see E5 Section 15.10.4.1).
 */

static duk_uint32_t parse_regexp_flags(duk_hthread *thr, duk_hstring *h) {
	duk_uint8_t *p;
	duk_uint8_t *p_end;
	duk_uint32_t flags = 0;

	p = DUK_HSTRING_GET_DATA(h);
	p_end = p + DUK_HSTRING_GET_BYTELEN(h);

	/* Note: can be safely scanned as bytes (undecoded) */

	while (p < p_end) {
		duk_uint8_t c = *p++;
		switch ((int) c) {
		case (int) 'g': {
			if (flags & DUK_RE_FLAG_GLOBAL) {
				goto error;
			}
			flags |= DUK_RE_FLAG_GLOBAL;
			break;
		}
		case (int) 'i': {
			if (flags & DUK_RE_FLAG_IGNORE_CASE) {
				goto error;
			}
			flags |= DUK_RE_FLAG_IGNORE_CASE;
			break;
		}
		case (int) 'm': {
			if (flags & DUK_RE_FLAG_MULTILINE) {
				goto error;
			}
			flags |= DUK_RE_FLAG_MULTILINE;
			break;
		}
		default: {
			goto error;
		}
		}
	}

	return flags;

 error:
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid regexp flags");
	return 0;  /* never here */
}

/*
 *  Create escaped RegExp source (E5 Section 15.10.3).
 *
 *  The current approach is to special case the empty RegExp
 *  ('' -> '(?:)') and otherwise replace unescaped '/' characters
 *  with '\/' regardless of where they occur in the regexp.
 *
 *  Note that normalization does not seem to be necessary for
 *  RegExp literals (e.g. '/foo/') because to be acceptable as
 *  a RegExp literal, the text between forward slashes must
 *  already match the escaping requirements (e.g. must not contain
 *  unescaped forward slashes or be empty).  Escaping IS needed
 *  for expressions like 'new Regexp("...", "")' however.
 *  Currently, we re-escape in either case.
 *
 *  Also note that we process the source here in UTF-8 encoded
 *  form.  This is correct, because any non-ASCII characters are
 *  passed through without change.
 */

static void create_escaped_source(duk_hthread *thr, int idx_pattern) {
	duk_context *ctx = (duk_context *) thr;
	duk_hstring *h;
	duk_hbuffer_dynamic *buf;
	const char *p;
	int i, n;
	char c_prev, c;

	h = duk_get_hstring(ctx, idx_pattern);
	DUK_ASSERT(h != NULL);
	p = (const char *) DUK_HSTRING_GET_DATA(h);
	n = DUK_HSTRING_GET_BYTELEN(h);

	if (n == 0) {
		/* return '(?:)' */
		duk_push_hstring_stridx(ctx, DUK_STRIDX_ESCAPED_EMPTY_REGEXP);
		return;
	}

	duk_push_dynamic_buffer(ctx, 0);
	buf = (duk_hbuffer_dynamic *) duk_get_hbuffer(ctx, -1);
	DUK_ASSERT(buf != NULL);

	c_prev = (char) 0;

	for (i = 0; i < n; i++) {
		c = p[i];

		if (c == '/' && c_prev != '\\') {
			/* Unescaped '/' ANYWHERE in the regexp (in disjunction,
			 * inside a character class, ...) => same escape works.
			 */
			duk_hbuffer_append_byte(thr, buf, (duk_uint8_t) '\\');
		}
		duk_hbuffer_append_byte(thr, buf, (duk_uint8_t) c);

		c_prev = c;
	}

	duk_to_string(ctx, -1);  /* -> [ ... escaped_source ] */
}

/*
 *  Exposed regexp compilation primitive.
 *
 *  Sets up a regexp compilation context, and calls parse_disjunction() to do the
 *  actual parsing.  Handles generation of the compiled regexp header and the
 *  "boilerplate" capture of the matching substring (save 0 and 1).  Also does some
 *  global level regexp checks after recursive compilation has finished.
 *
 *  An escaped version of the regexp source, suitable for use as a RegExp instance
 *  'source' property (see E5 Section 15.10.3), is also left on the stack.
 *
 *  Input stack:  [ pattern flags ]
 *  Output stack: [ bytecode escaped_source ]  (both as strings)
 */

void duk_regexp_compile(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_re_compiler_ctx re_ctx;
	duk_lexer_point lex_point;
	duk_hstring *h_pattern;
	duk_hstring *h_flags;
	duk_hbuffer_dynamic *h_buffer;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);

	/*
	 *  Args validation
	 */

	/* TypeError if fails */
	h_pattern = duk_require_hstring(ctx, -2);
	h_flags = duk_require_hstring(ctx, -1);

	/*
	 *  Create normalized 'source' property (E5 Section 15.10.3).
	 */

	/* [ ... pattern flags ] */

	create_escaped_source(thr, -2);

	/* [ ... pattern flags escaped_source ] */

	/*
	 *  Init compilation context
	 */

	duk_push_dynamic_buffer(ctx, 0);
	h_buffer = (duk_hbuffer_dynamic *) duk_require_hbuffer(ctx, -1);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(h_buffer));

	/* [ ... pattern flags escaped_source buffer ] */

	DUK_MEMSET(&re_ctx, 0, sizeof(re_ctx));
	DUK_LEXER_INITCTX(&re_ctx.lex);  /* duplicate zeroing, expect for (possible) NULL inits */
	re_ctx.thr = thr;
	re_ctx.lex.thr = thr;
	re_ctx.lex.input = DUK_HSTRING_GET_DATA(h_pattern);
	re_ctx.lex.input_length = DUK_HSTRING_GET_BYTELEN(h_pattern);
	re_ctx.buf = h_buffer;
	re_ctx.recursion_limit = DUK_RE_COMPILE_RECURSION_LIMIT;
	re_ctx.re_flags = parse_regexp_flags(thr, h_flags);

	DUK_DDPRINT("regexp compiler ctx initialized, flags=0x%08x, recursion_limit=%d",
	            re_ctx.re_flags, re_ctx.recursion_limit);

	/*
	 *  Init lexer
	 */

	lex_point.offset = 0;		/* expensive init, just want to fill window */
	lex_point.line = 1;
	DUK_LEXER_SETPOINT(&re_ctx.lex, &lex_point);

	/*
	 *  Compilation
	 */

	DUK_DPRINT("starting regexp compilation");

	append_u32(&re_ctx, DUK_REOP_SAVE);
	append_u32(&re_ctx, 0);
	(void) parse_disjunction(&re_ctx, 1);  /* 1 = expect eof */
	append_u32(&re_ctx, DUK_REOP_SAVE);
	append_u32(&re_ctx, 1);
	append_u32(&re_ctx, DUK_REOP_MATCH);

	DUK_DPRINT("regexp bytecode size (before header) is %d bytes",
	           DUK_HBUFFER_GET_SIZE(re_ctx.buf));

	/*
	 *  Check for invalid backreferences; note that it is NOT an error
	 *  to back-reference a capture group which has not yet been introduced
	 *  in the pattern (as in /\1(foo)/); in fact, the backreference will
	 *  always match!  It IS an error to back-reference a capture group
	 *  which will never be introduced in the pattern.  Thus, we can check
	 *  for such references only after parsing is complete.
	 */

	if (re_ctx.highest_backref > re_ctx.captures) {
		DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid backreference(s)");
	}

	/*
	 *  Emit compiled regexp header: flags, ncaptures
	 *  (insertion order inverted on purpose)
	 */

	insert_u32(&re_ctx, 0, (re_ctx.captures + 1) * 2);
	insert_u32(&re_ctx, 0, re_ctx.re_flags);

	DUK_DPRINT("regexp bytecode size (after header) is %d bytes",
	           DUK_HBUFFER_GET_SIZE(re_ctx.buf));
	DUK_DDDPRINT("compiled regexp: %!xO", re_ctx.buf);

	/* [ ... pattern flags escaped_source buffer ] */

	duk_to_string(ctx, -1);  /* coerce to string */

	/* [ ... pattern flags escaped_source bytecode ] */

	/*
	 *  Finalize stack
	 */

	duk_remove(ctx, -4);     /* -> [ ... flags escaped_source bytecode ] */
	duk_remove(ctx, -3);     /* -> [ ... escaped_source bytecode ] */

	DUK_DPRINT("regexp compilation successful, bytecode: %!T, escaped source: %!T",
	           duk_get_tval(ctx, -1), duk_get_tval(ctx, -2));
}

/*
 *  Create a RegExp instance (E5 Section 15.10.7).
 *
 *  Note: the output stack left by duk_regexp_compile() is directly compatible
 *  with the input here.
 *
 *  Input stack:  [ escaped_source bytecode ]  (both as strings)
 *  Output stack: [ RegExp ]
 */
 
void duk_regexp_create_instance(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_hobject *h;
	duk_hstring *h_bc;
	int re_flags;

	/* [ ... escape_source bytecode ] */

	h_bc = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h_bc != NULL);
	DUK_ASSERT(DUK_HSTRING_GET_BYTELEN(h_bc) >= 1);          /* always at least the header */
	DUK_ASSERT(DUK_HSTRING_GET_CHARLEN(h_bc) >= 1);
	DUK_ASSERT((int) DUK_HSTRING_GET_DATA(h_bc)[0] < 0x80);  /* flags always encodes to 1 byte */
	re_flags = (int) DUK_HSTRING_GET_DATA(h_bc)[0];

	/* [ ... escaped_source bytecode ] */

	duk_push_object(ctx);
	h = duk_get_hobject(ctx, -1);
	DUK_ASSERT(h != NULL);
	duk_insert(ctx, -3);

	/* [ ... regexp_object escaped_source bytecode ] */

	DUK_HOBJECT_SET_CLASS_NUMBER(h, DUK_HOBJECT_CLASS_REGEXP);
	DUK_HOBJECT_SET_PROTOTYPE(thr, h, thr->builtins[DUK_BIDX_REGEXP_PROTOTYPE]);

	duk_def_prop_stridx(ctx, -3, DUK_STRIDX_INT_BYTECODE, DUK_PROPDESC_FLAGS_NONE);

	/* [ ... regexp_object escaped_source ] */

	duk_def_prop_stridx(ctx, -2, DUK_STRIDX_SOURCE, DUK_PROPDESC_FLAGS_NONE);

	/* [ ... regexp_object ] */

	duk_push_boolean(ctx, (re_flags & DUK_RE_FLAG_GLOBAL));
	duk_def_prop_stridx(ctx, -2, DUK_STRIDX_GLOBAL, DUK_PROPDESC_FLAGS_NONE);

	duk_push_boolean(ctx, (re_flags & DUK_RE_FLAG_IGNORE_CASE));
	duk_def_prop_stridx(ctx, -2, DUK_STRIDX_IGNORE_CASE, DUK_PROPDESC_FLAGS_NONE);

	duk_push_boolean(ctx, (re_flags & DUK_RE_FLAG_MULTILINE));
	duk_def_prop_stridx(ctx, -2, DUK_STRIDX_MULTILINE, DUK_PROPDESC_FLAGS_NONE);

	duk_push_int(ctx, 0);
	duk_def_prop_stridx(ctx, -2, DUK_STRIDX_LAST_INDEX, DUK_PROPDESC_FLAGS_W);

	/* [ ... regexp_object ] */
}

#undef DUK_BUFLEN

#endif  /* DUK_USE_REGEXP_SUPPORT */

