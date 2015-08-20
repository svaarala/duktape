/*
 *  Regexp compilation.
 *
 *  See doc/regexp.rst for a discussion of the compilation approach and
 *  current limitations.
 *
 *  Regexp bytecode assumes jumps can be expressed with signed 32-bit
 *  integers.  Consequently the bytecode size must not exceed 0x7fffffffL.
 *  The implementation casts duk_size_t (buffer size) to duk_(u)int32_t
 *  in many places.  Although this could be changed, the bytecode format
 *  limit would still prevent regexps exceeding the signed 32-bit limit
 *  from working.
 *
 *  XXX: The implementation does not prevent bytecode from exceeding the
 *  maximum supported size.  This could be done by limiting the maximum
 *  input string size (assuming an upper bound can be computed for number
 *  of bytecode bytes emitted per input byte) or checking buffer maximum
 *  size when emitting bytecode (slower).
 */

#include "duk_internal.h"

#ifdef DUK_USE_REGEXP_SUPPORT

/*
 *  Helper macros
 */

#define DUK__RE_INITIAL_BUFSIZE 64

#undef DUK__RE_BUFLEN
#define DUK__RE_BUFLEN(re_ctx) \
	DUK_BW_GET_SIZE(re_ctx->thr, &re_ctx->bw)

/*
 *  Disjunction struct: result of parsing a disjunction
 */

typedef struct {
	/* Number of characters that the atom matches (e.g. 3 for 'abc'),
	 * -1 if atom is complex and number of matched characters either
	 * varies or is not known.
	 */
	duk_int32_t charlen;

#if 0
	/* These are not needed to implement quantifier capture handling,
	 * but might be needed at some point.
	 */

	/* re_ctx->captures at start and end of atom parsing.
	 * Since 'captures' indicates highest capture number emitted
	 * so far in a DUK_REOP_SAVE, the captures numbers saved by
	 * the atom are: ]start_captures,end_captures].
	 */
	duk_uint32_t start_captures;
	duk_uint32_t end_captures;
#endif
} duk__re_disjunction_info;

/*
 *  Encoding helpers
 *
 *  Some of the typing is bytecode based, e.g. slice sizes are unsigned 32-bit
 *  even though the buffer operations will use duk_size_t.
 */

/* XXX: the insert helpers should ensure that the bytecode result is not
 * larger than expected (or at least assert for it).  Many things in the
 * bytecode, like skip offsets, won't work correctly if the bytecode is
 * larger than say 2G.
 */

DUK_LOCAL duk_uint32_t duk__encode_i32(duk_int32_t x) {
	if (x < 0) {
		return ((duk_uint32_t) (-x)) * 2 + 1;
	} else {
		return ((duk_uint32_t) x) * 2;
	}
}

/* XXX: return type should probably be duk_size_t, or explicit checks are needed for
 * maximum size.
 */
DUK_LOCAL duk_uint32_t duk__insert_u32(duk_re_compiler_ctx *re_ctx, duk_uint32_t offset, duk_uint32_t x) {
	duk_uint8_t buf[DUK_UNICODE_MAX_XUTF8_LENGTH];
	duk_small_int_t len;

	len = duk_unicode_encode_xutf8((duk_ucodepoint_t) x, buf);
	DUK_BW_INSERT_ENSURE_BYTES(re_ctx->thr, &re_ctx->bw, offset, buf, len);
	return (duk_uint32_t) len;
}

DUK_LOCAL duk_uint32_t duk__append_u32(duk_re_compiler_ctx *re_ctx, duk_uint32_t x) {
	duk_uint8_t buf[DUK_UNICODE_MAX_XUTF8_LENGTH];
	duk_small_int_t len;

	len = duk_unicode_encode_xutf8((duk_ucodepoint_t) x, buf);
	DUK_BW_WRITE_ENSURE_BYTES(re_ctx->thr, &re_ctx->bw, buf, len);
	return (duk_uint32_t) len;
}

DUK_LOCAL duk_uint32_t duk__insert_i32(duk_re_compiler_ctx *re_ctx, duk_uint32_t offset, duk_int32_t x) {
	return duk__insert_u32(re_ctx, offset, duk__encode_i32(x));
}

#if 0  /* unused */
DUK_LOCAL duk_uint32_t duk__append_i32(duk_re_compiler_ctx *re_ctx, duk_int32_t x) {
	return duk__append_u32(re_ctx, duk__encode_i32(x));
}
#endif

/* special helper for emitting u16 lists (used for character ranges for built-in char classes) */
DUK_LOCAL void duk__append_u16_list(duk_re_compiler_ctx *re_ctx, duk_uint16_t *values, duk_uint32_t count) {
	/* Call sites don't need the result length so it's not accumulated. */
	while (count > 0) {
		(void) duk__append_u32(re_ctx, (duk_uint32_t) (*values++));
		count--;
	}
}

DUK_LOCAL void duk__insert_slice(duk_re_compiler_ctx *re_ctx, duk_uint32_t offset, duk_uint32_t data_offset, duk_uint32_t data_length) {
	DUK_BW_INSERT_ENSURE_SLICE(re_ctx->thr, &re_ctx->bw, offset, data_offset, data_length);
}

DUK_LOCAL void duk__append_slice(duk_re_compiler_ctx *re_ctx, duk_uint32_t data_offset, duk_uint32_t data_length) {
	DUK_BW_WRITE_ENSURE_SLICE(re_ctx->thr, &re_ctx->bw, data_offset, data_length);
}

DUK_LOCAL void duk__remove_slice(duk_re_compiler_ctx *re_ctx, duk_uint32_t data_offset, duk_uint32_t data_length) {
	DUK_BW_REMOVE_ENSURE_SLICE(re_ctx->thr, &re_ctx->bw, data_offset, data_length);
}

/*
 *  Insert a jump offset at 'offset' to complete an instruction
 *  (the jump offset is always the last component of an instruction).
 *  The 'skip' argument must be computed relative to 'offset',
 *  -without- taking into account the skip field being inserted.
 *
 *       ... A B C ins X Y Z ...   (ins may be a JUMP, SPLIT1/SPLIT2, etc)
 *   =>  ... A B C ins SKIP X Y Z
 *
 *  Computing the final (adjusted) skip value, which is relative to the
 *  first byte of the next instruction, is a bit tricky because of the
 *  variable length UTF-8 encoding.  See doc/regexp.rst for discussion.
 */
DUK_LOCAL duk_uint32_t duk__insert_jump_offset(duk_re_compiler_ctx *re_ctx, duk_uint32_t offset, duk_int32_t skip) {
	duk_small_int_t len;

	/* XXX: solve into closed form (smaller code) */

	if (skip < 0) {
		/* two encoding attempts suffices */
		len = duk_unicode_get_xutf8_length((duk_codepoint_t) duk__encode_i32(skip));
		len = duk_unicode_get_xutf8_length((duk_codepoint_t) duk__encode_i32(skip - (duk_int32_t) len));
		DUK_ASSERT(duk_unicode_get_xutf8_length(duk__encode_i32(skip - (duk_int32_t) len)) == len);  /* no change */
		skip -= (duk_int32_t) len;
	}
	return duk__insert_i32(re_ctx, offset, skip);
}

DUK_LOCAL duk_uint32_t duk__append_jump_offset(duk_re_compiler_ctx *re_ctx, duk_int32_t skip) {
	return (duk_uint32_t) duk__insert_jump_offset(re_ctx, (duk_uint32_t) DUK__RE_BUFLEN(re_ctx), skip);
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
 *  time execution time (see doc/regexp.rst for discussion).
 *
 *  Note that the ctx->nranges is a context-wide temporary value
 *  (this is OK because there cannot be multiple character classes
 *  being parsed simultaneously).
 */

DUK_LOCAL void duk__generate_ranges(void *userdata, duk_codepoint_t r1, duk_codepoint_t r2, duk_bool_t direct) {
	duk_re_compiler_ctx *re_ctx = (duk_re_compiler_ctx *) userdata;

	DUK_DD(DUK_DDPRINT("duk__generate_ranges(): re_ctx=%p, range=[%ld,%ld] direct=%ld",
	                   (void *) re_ctx, (long) r1, (long) r2, (long) direct));

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

		duk_codepoint_t i;
		duk_codepoint_t t;
		duk_codepoint_t r_start, r_end;

		r_start = duk_unicode_re_canonicalize_char(re_ctx->thr, r1);
		r_end = r_start;
		for (i = r1 + 1; i <= r2; i++) {
			t = duk_unicode_re_canonicalize_char(re_ctx->thr, i);
			if (t == r_end + 1) {
				r_end = t;
			} else {
				DUK_DD(DUK_DDPRINT("canonicalized, emit range: [%ld,%ld]", (long) r_start, (long) r_end));
				duk__append_u32(re_ctx, (duk_uint32_t) r_start);
				duk__append_u32(re_ctx, (duk_uint32_t) r_end);
				re_ctx->nranges++;
				r_start = t;
				r_end = t;
			}
		}
		DUK_DD(DUK_DDPRINT("canonicalized, emit range: [%ld,%ld]", (long) r_start, (long) r_end));
		duk__append_u32(re_ctx, (duk_uint32_t) r_start);
		duk__append_u32(re_ctx, (duk_uint32_t) r_end);
		re_ctx->nranges++;
	} else {
		DUK_DD(DUK_DDPRINT("direct, emit range: [%ld,%ld]", (long) r1, (long) r2));
		duk__append_u32(re_ctx, (duk_uint32_t) r1);
		duk__append_u32(re_ctx, (duk_uint32_t) r2);
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
 *  (see doc/regexp.rst discussion on 'simple quantifiers') and if so,
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

DUK_LOCAL void duk__parse_disjunction(duk_re_compiler_ctx *re_ctx, duk_bool_t expect_eof, duk__re_disjunction_info *out_atom_info) {
	duk_int32_t atom_start_offset = -1;                   /* negative -> no atom matched on previous round */
	duk_int32_t atom_char_length = 0;                     /* negative -> complex atom */
	duk_uint32_t atom_start_captures = re_ctx->captures;  /* value of re_ctx->captures at start of atom */
	duk_int32_t unpatched_disjunction_split = -1;
	duk_int32_t unpatched_disjunction_jump = -1;
	duk_uint32_t entry_offset = (duk_uint32_t) DUK__RE_BUFLEN(re_ctx);
	duk_int32_t res_charlen = 0;  /* -1 if disjunction is complex, char length if simple */
	duk__re_disjunction_info tmp_disj;

	DUK_ASSERT(out_atom_info != NULL);

	if (re_ctx->recursion_depth >= re_ctx->recursion_limit) {
		DUK_ERROR(re_ctx->thr, DUK_ERR_RANGE_ERROR,
		          DUK_STR_REGEXP_COMPILER_RECURSION_LIMIT);
	}
	re_ctx->recursion_depth++;

#if 0
	out_atom_info->start_captures = re_ctx->captures;
#endif

	for (;;) {
		/* atom_char_length, atom_start_offset, atom_start_offset reflect the
		 * atom matched on the previous loop.  If a quantifier is encountered
		 * on this loop, these are needed to handle the quantifier correctly.
		 * new_atom_char_length etc are for the atom parsed on this round;
		 * they're written to atom_char_length etc at the end of the round.
		 */
		duk_int32_t new_atom_char_length;   /* char length of the atom parsed in this loop */
		duk_int32_t new_atom_start_offset;  /* bytecode start offset of the atom parsed in this loop
		                                     * (allows quantifiers to copy the atom bytecode)
		                                     */
		duk_uint32_t new_atom_start_captures;  /* re_ctx->captures at the start of the atom parsed in this loop */

		duk_lexer_parse_re_token(&re_ctx->lex, &re_ctx->curr_token);

		DUK_DD(DUK_DDPRINT("re token: %ld (num=%ld, char=%c)",
		                   (long) re_ctx->curr_token.t,
		                   (long) re_ctx->curr_token.num,
		                   (re_ctx->curr_token.num >= 0x20 && re_ctx->curr_token.num <= 0x7e) ?
		                   (int) re_ctx->curr_token.num : (int) '?'));

		/* set by atom case clauses */
		new_atom_start_offset = -1;
		new_atom_char_length = -1;
		new_atom_start_captures = re_ctx->captures;

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
				offset += duk__insert_jump_offset(re_ctx,
				                                  offset,
				                                  (duk_int32_t) (DUK__RE_BUFLEN(re_ctx) - offset));
				/* offset is now target of the pending split (right after jump) */
				duk__insert_jump_offset(re_ctx,
				                        unpatched_disjunction_split,
				                        offset - unpatched_disjunction_split);
			}

			/* add a new pending split to the beginning of the entire disjunction */
			(void) duk__insert_u32(re_ctx,
			                       entry_offset,
			                       DUK_REOP_SPLIT1);   /* prefer direct execution */
			unpatched_disjunction_split = entry_offset + 1;   /* +1 for opcode */

			/* add a new pending match jump for latest finished alternative */
			duk__append_u32(re_ctx, DUK_REOP_JUMP);
			unpatched_disjunction_jump = (duk_int32_t) DUK__RE_BUFLEN(re_ctx);

			/* 'taint' result as complex */
			res_charlen = -1;
			break;
		}
		case DUK_RETOK_QUANTIFIER: {
			if (atom_start_offset < 0) {
				DUK_ERROR(re_ctx->thr, DUK_ERR_SYNTAX_ERROR,
				          DUK_STR_INVALID_QUANTIFIER_NO_ATOM);
			}
			if (re_ctx->curr_token.qmin > re_ctx->curr_token.qmax) {
				DUK_ERROR(re_ctx->thr, DUK_ERR_SYNTAX_ERROR,
				          DUK_STR_INVALID_QUANTIFIER_VALUES);
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
				 *
				 *  With a simple atom there can be no capture groups, so no captures need
				 *  to be reset.
				 */
				duk_int32_t atom_code_length;
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

				duk__append_u32(re_ctx, DUK_REOP_MATCH);   /* complete 'sub atom' */
				atom_code_length = (duk_int32_t) (DUK__RE_BUFLEN(re_ctx) - atom_start_offset);

				offset = atom_start_offset;
				if (re_ctx->curr_token.greedy) {
					offset += duk__insert_u32(re_ctx, offset, DUK_REOP_SQGREEDY);
					offset += duk__insert_u32(re_ctx, offset, qmin);
					offset += duk__insert_u32(re_ctx, offset, qmax);
					offset += duk__insert_u32(re_ctx, offset, atom_char_length);
					offset += duk__insert_jump_offset(re_ctx, offset, atom_code_length);
				} else {
					offset += duk__insert_u32(re_ctx, offset, DUK_REOP_SQMINIMAL);
					offset += duk__insert_u32(re_ctx, offset, qmin);
					offset += duk__insert_u32(re_ctx, offset, qmax);
					offset += duk__insert_jump_offset(re_ctx, offset, atom_code_length);
				}
				DUK_UNREF(offset);  /* silence scan-build warning */
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
				duk_uint32_t atom_copies;
				duk_uint32_t tmp_qmin, tmp_qmax;

				/* pre-check how many atom copies we're willing to make (atom_copies not needed below) */
				atom_copies = (re_ctx->curr_token.qmax == DUK_RE_QUANTIFIER_INFINITE) ?
				              re_ctx->curr_token.qmin : re_ctx->curr_token.qmax;
				if (atom_copies > DUK_RE_MAX_ATOM_COPIES) {
					DUK_ERROR(re_ctx->thr, DUK_ERR_RANGE_ERROR,
					          DUK_STR_QUANTIFIER_TOO_MANY_COPIES);
				}

				/* wipe the capture range made by the atom (if any) */
				DUK_ASSERT(atom_start_captures <= re_ctx->captures);
				if (atom_start_captures != re_ctx->captures) {
					DUK_ASSERT(atom_start_captures < re_ctx->captures);
					DUK_DDD(DUK_DDDPRINT("must wipe ]atom_start_captures,re_ctx->captures]: ]%ld,%ld]",
					                     (long) atom_start_captures, (long) re_ctx->captures));

					/* insert (DUK_REOP_WIPERANGE, start, count) in reverse order so the order ends up right */
					duk__insert_u32(re_ctx, atom_start_offset, (re_ctx->captures - atom_start_captures) * 2);
					duk__insert_u32(re_ctx, atom_start_offset, (atom_start_captures + 1) * 2);
					duk__insert_u32(re_ctx, atom_start_offset, DUK_REOP_WIPERANGE);
				} else {
					DUK_DDD(DUK_DDDPRINT("no need to wipe captures: atom_start_captures == re_ctx->captures == %ld",
					                     (long) atom_start_captures));
				}

				atom_code_length = (duk_int32_t) DUK__RE_BUFLEN(re_ctx) - atom_start_offset;

				/* insert the required matches (qmin) by copying the atom */
				tmp_qmin = re_ctx->curr_token.qmin;
				tmp_qmax = re_ctx->curr_token.qmax;
				while (tmp_qmin > 0) {
					duk__append_slice(re_ctx, atom_start_offset, atom_code_length);
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
						duk__append_u32(re_ctx, DUK_REOP_JUMP);
						duk__append_jump_offset(re_ctx, atom_code_length);
						duk__append_slice(re_ctx, atom_start_offset, atom_code_length);
					}
					if (re_ctx->curr_token.greedy) {
						duk__append_u32(re_ctx, DUK_REOP_SPLIT2);   /* prefer jump */
					} else {
						duk__append_u32(re_ctx, DUK_REOP_SPLIT1);   /* prefer direct */
					}
					duk__append_jump_offset(re_ctx, -atom_code_length - 1);  /* -1 for opcode */
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
					 *      (atom)         ; to encode the above SPLIT1 correctly
					 *      ...
					 *   LSEQ:
					 */
					duk_uint32_t offset = (duk_uint32_t) DUK__RE_BUFLEN(re_ctx);
					while (tmp_qmax > 0) {
						duk__insert_slice(re_ctx, offset, atom_start_offset, atom_code_length);
						if (re_ctx->curr_token.greedy) {
							duk__insert_u32(re_ctx, offset, DUK_REOP_SPLIT1);   /* prefer direct */
						} else {
							duk__insert_u32(re_ctx, offset, DUK_REOP_SPLIT2);   /* prefer jump */
						}
						duk__insert_jump_offset(re_ctx,
						                        offset + 1,   /* +1 for opcode */
						                        (duk_int32_t) (DUK__RE_BUFLEN(re_ctx) - (offset + 1)));
						tmp_qmax--;
					}
				}

				/* remove the original 'template' atom */
				duk__remove_slice(re_ctx, atom_start_offset, atom_code_length);
			}

			/* 'taint' result as complex */
			res_charlen = -1;
			break;
		}
		case DUK_RETOK_ASSERT_START: {
			duk__append_u32(re_ctx, DUK_REOP_ASSERT_START);
			break;
		}
		case DUK_RETOK_ASSERT_END: {
			duk__append_u32(re_ctx, DUK_REOP_ASSERT_END);
			break;
		}
		case DUK_RETOK_ASSERT_WORD_BOUNDARY: {
			duk__append_u32(re_ctx, DUK_REOP_ASSERT_WORD_BOUNDARY);
			break;
		}
		case DUK_RETOK_ASSERT_NOT_WORD_BOUNDARY: {
			duk__append_u32(re_ctx, DUK_REOP_ASSERT_NOT_WORD_BOUNDARY);
			break;
		}
		case DUK_RETOK_ASSERT_START_POS_LOOKAHEAD:
		case DUK_RETOK_ASSERT_START_NEG_LOOKAHEAD: {
			duk_uint32_t offset;
			duk_uint32_t opcode = (re_ctx->curr_token.t == DUK_RETOK_ASSERT_START_POS_LOOKAHEAD) ?
			                      DUK_REOP_LOOKPOS : DUK_REOP_LOOKNEG;

			offset = (duk_uint32_t) DUK__RE_BUFLEN(re_ctx);
			duk__parse_disjunction(re_ctx, 0, &tmp_disj);
			duk__append_u32(re_ctx, DUK_REOP_MATCH);

			(void) duk__insert_u32(re_ctx, offset, opcode);
			(void) duk__insert_jump_offset(re_ctx,
			                               offset + 1,   /* +1 for opcode */
			                               (duk_int32_t) (DUK__RE_BUFLEN(re_ctx) - (offset + 1)));

			/* 'taint' result as complex -- this is conservative,
			 * as lookaheads do not backtrack.
			 */
			res_charlen = -1;
			break;
		}
		case DUK_RETOK_ATOM_PERIOD: {
			new_atom_char_length = 1;
			new_atom_start_offset = (duk_int32_t) DUK__RE_BUFLEN(re_ctx);
			duk__append_u32(re_ctx, DUK_REOP_PERIOD);
			break;
		}
		case DUK_RETOK_ATOM_CHAR: {
			/* Note: successive characters could be joined into string matches
			 * but this is not trivial (consider e.g. '/xyz+/); see docs for
			 * more discussion.
			 */
			duk_uint32_t ch;

			new_atom_char_length = 1;
			new_atom_start_offset = (duk_int32_t) DUK__RE_BUFLEN(re_ctx);
			duk__append_u32(re_ctx, DUK_REOP_CHAR);
			ch = re_ctx->curr_token.num;
			if (re_ctx->re_flags & DUK_RE_FLAG_IGNORE_CASE) {
				ch = duk_unicode_re_canonicalize_char(re_ctx->thr, ch);
			}
			duk__append_u32(re_ctx, ch);
			break;
		}
		case DUK_RETOK_ATOM_DIGIT:
		case DUK_RETOK_ATOM_NOT_DIGIT: {
			new_atom_char_length = 1;
			new_atom_start_offset = (duk_int32_t) DUK__RE_BUFLEN(re_ctx);
			duk__append_u32(re_ctx,
			                (re_ctx->curr_token.t == DUK_RETOK_ATOM_DIGIT) ?
			                DUK_REOP_RANGES : DUK_REOP_INVRANGES);
			duk__append_u32(re_ctx, sizeof(duk_unicode_re_ranges_digit) / (2 * sizeof(duk_uint16_t)));
			duk__append_u16_list(re_ctx, duk_unicode_re_ranges_digit, sizeof(duk_unicode_re_ranges_digit) / sizeof(duk_uint16_t));
			break;
		}
		case DUK_RETOK_ATOM_WHITE:
		case DUK_RETOK_ATOM_NOT_WHITE: {
			new_atom_char_length = 1;
			new_atom_start_offset = (duk_int32_t) DUK__RE_BUFLEN(re_ctx);
			duk__append_u32(re_ctx,
			                (re_ctx->curr_token.t == DUK_RETOK_ATOM_WHITE) ?
			                DUK_REOP_RANGES : DUK_REOP_INVRANGES);
			duk__append_u32(re_ctx, sizeof(duk_unicode_re_ranges_white) / (2 * sizeof(duk_uint16_t)));
			duk__append_u16_list(re_ctx, duk_unicode_re_ranges_white, sizeof(duk_unicode_re_ranges_white) / sizeof(duk_uint16_t));
			break;
		}
		case DUK_RETOK_ATOM_WORD_CHAR:
		case DUK_RETOK_ATOM_NOT_WORD_CHAR: {
			new_atom_char_length = 1;
			new_atom_start_offset = (duk_int32_t) DUK__RE_BUFLEN(re_ctx);
			duk__append_u32(re_ctx,
			                (re_ctx->curr_token.t == DUK_RETOK_ATOM_WORD_CHAR) ?
			                DUK_REOP_RANGES : DUK_REOP_INVRANGES);
			duk__append_u32(re_ctx, sizeof(duk_unicode_re_ranges_wordchar) / (2 * sizeof(duk_uint16_t)));
			duk__append_u16_list(re_ctx, duk_unicode_re_ranges_wordchar, sizeof(duk_unicode_re_ranges_wordchar) / sizeof(duk_uint16_t));
			break;
		}
		case DUK_RETOK_ATOM_BACKREFERENCE: {
			duk_uint32_t backref = (duk_uint32_t) re_ctx->curr_token.num;
			if (backref > re_ctx->highest_backref) {
				re_ctx->highest_backref = backref;
			}
			new_atom_char_length = -1;   /* mark as complex */
			new_atom_start_offset = (duk_int32_t) DUK__RE_BUFLEN(re_ctx);
			duk__append_u32(re_ctx, DUK_REOP_BACKREFERENCE);
			duk__append_u32(re_ctx, backref);
			break;
		}
		case DUK_RETOK_ATOM_START_CAPTURE_GROUP: {
			duk_uint32_t cap;

			new_atom_char_length = -1;   /* mark as complex (capture handling) */
			new_atom_start_offset = (duk_int32_t) DUK__RE_BUFLEN(re_ctx);
			cap = ++re_ctx->captures;
			duk__append_u32(re_ctx, DUK_REOP_SAVE);
			duk__append_u32(re_ctx, cap * 2);
			duk__parse_disjunction(re_ctx, 0, &tmp_disj);  /* retval (sub-atom char length) unused, tainted as complex above */
			duk__append_u32(re_ctx, DUK_REOP_SAVE);
			duk__append_u32(re_ctx, cap * 2 + 1);
			break;
		}
		case DUK_RETOK_ATOM_START_NONCAPTURE_GROUP: {
			new_atom_start_offset = (duk_int32_t) DUK__RE_BUFLEN(re_ctx);
			duk__parse_disjunction(re_ctx, 0, &tmp_disj);
			new_atom_char_length = tmp_disj.charlen;
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

			DUK_DD(DUK_DDPRINT("character class"));

			/* insert ranges instruction, range count patched in later */
			new_atom_char_length = 1;
			new_atom_start_offset = (duk_int32_t) DUK__RE_BUFLEN(re_ctx);
			duk__append_u32(re_ctx,
			                (re_ctx->curr_token.t == DUK_RETOK_ATOM_START_CHARCLASS) ?
			                DUK_REOP_RANGES : DUK_REOP_INVRANGES);
			offset = (duk_uint32_t) DUK__RE_BUFLEN(re_ctx);    /* patch in range count later */

			/* parse ranges until character class ends */
			re_ctx->nranges = 0;    /* note: ctx-wide temporary */
			duk_lexer_parse_re_ranges(&re_ctx->lex, duk__generate_ranges, (void *) re_ctx);

			/* insert range count */
			duk__insert_u32(re_ctx, offset, re_ctx->nranges);
			break;
		}
		case DUK_RETOK_ATOM_END_GROUP: {
			if (expect_eof) {
				DUK_ERROR(re_ctx->thr, DUK_ERR_SYNTAX_ERROR,
				          DUK_STR_UNEXPECTED_CLOSING_PAREN);
			}
			goto done;
		}
		case DUK_RETOK_EOF: {
			if (!expect_eof) {
				DUK_ERROR(re_ctx->thr, DUK_ERR_SYNTAX_ERROR,
				          DUK_STR_UNEXPECTED_END_OF_PATTERN);
			}
			goto done;
		}
		default: {
			DUK_ERROR(re_ctx->thr, DUK_ERR_SYNTAX_ERROR,
			          DUK_STR_UNEXPECTED_REGEXP_TOKEN);
		}
		}

		/* a complex (new) atom taints the result */
		if (new_atom_start_offset >= 0) {
			if (new_atom_char_length < 0) {
				res_charlen = -1;
			} else if (res_charlen >= 0) {
				/* only advance if not tainted */
				res_charlen += new_atom_char_length;
			}
		}

		/* record previous atom info in case next token is a quantifier */
		atom_start_offset = new_atom_start_offset;
		atom_char_length = new_atom_char_length;
		atom_start_captures = new_atom_start_captures;
	}

 done:

	/* finish up pending jump and split for last alternative */
	if (unpatched_disjunction_jump >= 0) {
		duk_uint32_t offset;

		DUK_ASSERT(unpatched_disjunction_split >= 0);
		offset = unpatched_disjunction_jump;
		offset += duk__insert_jump_offset(re_ctx,
		                                  offset,
		                                  (duk_int32_t) (DUK__RE_BUFLEN(re_ctx) - offset));
		/* offset is now target of the pending split (right after jump) */
		duk__insert_jump_offset(re_ctx,
		                        unpatched_disjunction_split,
		                        offset - unpatched_disjunction_split);
	}

#if 0
	out_atom_info->end_captures = re_ctx->captures;
#endif
	out_atom_info->charlen = res_charlen;
	DUK_DDD(DUK_DDDPRINT("parse disjunction finished: charlen=%ld",
	                     (long) out_atom_info->charlen));

	re_ctx->recursion_depth--;
}

/*
 *  Flags parsing (see E5 Section 15.10.4.1).
 */

DUK_LOCAL duk_uint32_t duk__parse_regexp_flags(duk_hthread *thr, duk_hstring *h) {
	const duk_uint8_t *p;
	const duk_uint8_t *p_end;
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
	DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, DUK_STR_INVALID_REGEXP_FLAGS);
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

DUK_LOCAL void duk__create_escaped_source(duk_hthread *thr, int idx_pattern) {
	duk_context *ctx = (duk_context *) thr;
	duk_hstring *h;
	const duk_uint8_t *p;
	duk_bufwriter_ctx bw_alloc;
	duk_bufwriter_ctx *bw;
	duk_uint8_t *q;
	duk_size_t i, n;
	duk_uint_fast8_t c_prev, c;

	h = duk_get_hstring(ctx, idx_pattern);
	DUK_ASSERT(h != NULL);
	p = (const duk_uint8_t *) DUK_HSTRING_GET_DATA(h);
	n = (duk_size_t) DUK_HSTRING_GET_BYTELEN(h);

	if (n == 0) {
		/* return '(?:)' */
		duk_push_hstring_stridx(ctx, DUK_STRIDX_ESCAPED_EMPTY_REGEXP);
		return;
	}

	bw = &bw_alloc;
	DUK_BW_INIT_PUSHBUF(thr, bw, n);
	q = DUK_BW_GET_PTR(thr, bw);

	c_prev = (duk_uint_fast8_t) 0;

	for (i = 0; i < n; i++) {
		c = p[i];

		q = DUK_BW_ENSURE_RAW(thr, bw, 2, q);

		if (c == (duk_uint_fast8_t) '/' && c_prev != (duk_uint_fast8_t) '\\') {
			/* Unescaped '/' ANYWHERE in the regexp (in disjunction,
			 * inside a character class, ...) => same escape works.
			 */
			*q++ = DUK_ASC_BACKSLASH;
		}
		*q++ = (duk_uint8_t) c;

		c_prev = c;
	}

	DUK_BW_SETPTR_AND_COMPACT(thr, bw, q);
	duk_to_string(ctx, -1);  /* -> [ ... escaped_source ] */
}

/*
 *  Exposed regexp compilation primitive.
 *
 *  Sets up a regexp compilation context, and calls duk__parse_disjunction() to do the
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

DUK_INTERNAL void duk_regexp_compile(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_re_compiler_ctx re_ctx;
	duk_lexer_point lex_point;
	duk_hstring *h_pattern;
	duk_hstring *h_flags;
	duk__re_disjunction_info ign_disj;

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

	duk__create_escaped_source(thr, -2);

	/* [ ... pattern flags escaped_source ] */

	/*
	 *  Init compilation context
	 */

	/* [ ... pattern flags escaped_source buffer ] */

	DUK_MEMZERO(&re_ctx, sizeof(re_ctx));
	DUK_LEXER_INITCTX(&re_ctx.lex);  /* duplicate zeroing, expect for (possible) NULL inits */
	re_ctx.thr = thr;
	re_ctx.lex.thr = thr;
	re_ctx.lex.input = DUK_HSTRING_GET_DATA(h_pattern);
	re_ctx.lex.input_length = DUK_HSTRING_GET_BYTELEN(h_pattern);
	re_ctx.lex.token_limit = DUK_RE_COMPILE_TOKEN_LIMIT;
	re_ctx.recursion_limit = DUK_USE_REGEXP_COMPILER_RECLIMIT;
	re_ctx.re_flags = duk__parse_regexp_flags(thr, h_flags);

	DUK_BW_INIT_PUSHBUF(thr, &re_ctx.bw, DUK__RE_INITIAL_BUFSIZE);

	DUK_DD(DUK_DDPRINT("regexp compiler ctx initialized, flags=0x%08lx, recursion_limit=%ld",
	                   (unsigned long) re_ctx.re_flags, (long) re_ctx.recursion_limit));

	/*
	 *  Init lexer
	 */

	lex_point.offset = 0;  /* expensive init, just want to fill window */
	lex_point.line = 1;
	DUK_LEXER_SETPOINT(&re_ctx.lex, &lex_point);

	/*
	 *  Compilation
	 */

	DUK_D(DUK_DPRINT("starting regexp compilation"));

	duk__append_u32(&re_ctx, DUK_REOP_SAVE);
	duk__append_u32(&re_ctx, 0);
	duk__parse_disjunction(&re_ctx, 1 /*expect_eof*/, &ign_disj);
	duk__append_u32(&re_ctx, DUK_REOP_SAVE);
	duk__append_u32(&re_ctx, 1);
	duk__append_u32(&re_ctx, DUK_REOP_MATCH);

	/*
	 *  Check for invalid backreferences; note that it is NOT an error
	 *  to back-reference a capture group which has not yet been introduced
	 *  in the pattern (as in /\1(foo)/); in fact, the backreference will
	 *  always match!  It IS an error to back-reference a capture group
	 *  which will never be introduced in the pattern.  Thus, we can check
	 *  for such references only after parsing is complete.
	 */

	if (re_ctx.highest_backref > re_ctx.captures) {
		DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, DUK_STR_INVALID_BACKREFS);
	}

	/*
	 *  Emit compiled regexp header: flags, ncaptures
	 *  (insertion order inverted on purpose)
	 */

	duk__insert_u32(&re_ctx, 0, (re_ctx.captures + 1) * 2);
	duk__insert_u32(&re_ctx, 0, re_ctx.re_flags);

	/* [ ... pattern flags escaped_source buffer ] */

	DUK_BW_COMPACT(thr, &re_ctx.bw);
	duk_to_string(ctx, -1);  /* coerce to string */

	/* [ ... pattern flags escaped_source bytecode ] */

	/*
	 *  Finalize stack
	 */

	duk_remove(ctx, -4);     /* -> [ ... flags escaped_source bytecode ] */
	duk_remove(ctx, -3);     /* -> [ ... escaped_source bytecode ] */

	DUK_D(DUK_DPRINT("regexp compilation successful, bytecode: %!T, escaped source: %!T",
	                 (duk_tval *) duk_get_tval(ctx, -1), (duk_tval *) duk_get_tval(ctx, -2)));
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

DUK_INTERNAL void duk_regexp_create_instance(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_hobject *h;
	duk_hstring *h_bc;
	duk_small_int_t re_flags;

	/* [ ... escape_source bytecode ] */

	h_bc = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h_bc != NULL);
	DUK_ASSERT(DUK_HSTRING_GET_BYTELEN(h_bc) >= 1);          /* always at least the header */
	DUK_ASSERT(DUK_HSTRING_GET_CHARLEN(h_bc) >= 1);
	DUK_ASSERT((duk_small_int_t) DUK_HSTRING_GET_DATA(h_bc)[0] < 0x80);  /* flags always encodes to 1 byte */
	re_flags = (duk_small_int_t) DUK_HSTRING_GET_DATA(h_bc)[0];

	/* [ ... escaped_source bytecode ] */

	duk_push_object(ctx);
	h = duk_get_hobject(ctx, -1);
	DUK_ASSERT(h != NULL);
	duk_insert(ctx, -3);

	/* [ ... regexp_object escaped_source bytecode ] */

	DUK_HOBJECT_SET_CLASS_NUMBER(h, DUK_HOBJECT_CLASS_REGEXP);
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, h, thr->builtins[DUK_BIDX_REGEXP_PROTOTYPE]);

	duk_xdef_prop_stridx(ctx, -3, DUK_STRIDX_INT_BYTECODE, DUK_PROPDESC_FLAGS_NONE);

	/* [ ... regexp_object escaped_source ] */

	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_SOURCE, DUK_PROPDESC_FLAGS_NONE);

	/* [ ... regexp_object ] */

	duk_push_boolean(ctx, (re_flags & DUK_RE_FLAG_GLOBAL));
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_GLOBAL, DUK_PROPDESC_FLAGS_NONE);

	duk_push_boolean(ctx, (re_flags & DUK_RE_FLAG_IGNORE_CASE));
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_IGNORE_CASE, DUK_PROPDESC_FLAGS_NONE);

	duk_push_boolean(ctx, (re_flags & DUK_RE_FLAG_MULTILINE));
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_MULTILINE, DUK_PROPDESC_FLAGS_NONE);

	duk_push_int(ctx, 0);
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_LAST_INDEX, DUK_PROPDESC_FLAGS_W);

	/* [ ... regexp_object ] */
}

#undef DUK__RE_BUFLEN

#else  /* DUK_USE_REGEXP_SUPPORT */

/* regexp support disabled */

#endif  /* DUK_USE_REGEXP_SUPPORT */
