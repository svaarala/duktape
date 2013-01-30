/*
 *  Regexp executor.
 *
 *  Safety: the Ecmascript executor should prevent user from reading and
 *  replacing regexp bytecode.  Even so, the executor must validate all
 *  memory accesses etc.  When an invalid access is detected (e.g. a 'save'
 *  opcode to invalid, unallocated index) it should fail with an internal
 *  error but not cause a segmentation fault.
 */

#include "duk_internal.h"

#ifdef DUK_USE_REGEXP_SUPPORT

/*
 *  Helpers for UTF-8 handling
 */

static duk_u32 bc_get_u32(duk_re_matcher_ctx *re_ctx, duk_u8 **pc) {
	return duk_unicode_xutf8_get_u32(re_ctx->thr, pc, re_ctx->bytecode, re_ctx->bytecode_end);
}

static duk_i32 bc_get_i32(duk_re_matcher_ctx *re_ctx, duk_u8 **pc) {
	duk_u32 t;

	/* signed integer encoding needed to work with UTF-8 */
	t = duk_unicode_xutf8_get_u32(re_ctx->thr, pc, re_ctx->bytecode, re_ctx->bytecode_end);
	if (t & 1) {
		return -(t >> 1);
	} else {
		return (t >> 1);
	}
}

static duk_u8 *utf8_backtrack(duk_hthread *thr, duk_u8 **ptr, duk_u8 *ptr_start, duk_u8 *ptr_end, duk_u32 count) {
	duk_u8 *p;

	/* Note: allow backtracking from p == ptr_end */
	p = *ptr;
	if (p < ptr_start || p > ptr_end) {
		goto fail;
	}

	while (count > 0) {
		for (;;) {
			p--;
			if (p < ptr_start) {
				goto fail;
			}
			if ((*p & 0xc0) != 0x80) {
				/* utf-8 continuation bytes have the form 10xx xxxx */
				break;
			}
		}
		count--;
	}
	*ptr = p;
	return p;

 fail:
	DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "regexp backtrack failed");
	return NULL;  /* never here */
}

static duk_u8 *utf8_advance(duk_hthread *thr, duk_u8 **ptr, duk_u8 *ptr_start, duk_u8 *ptr_end, duk_u32 count) {
	duk_u8 *p;

	p = *ptr;
	if (p < ptr_start || p >= ptr_end) {
		goto fail;
	}

	while (count > 0) {
		for (;;) {
			p++;

			/* Note: if encoding ends by hitting end of input, we don't check that
			 * the encoding is valid, we just assume it is.
			 */
			if (p >= ptr_end || ((*p & 0xc0) != 0x80)) {
				/* utf-8 continuation bytes have the form 10xx xxxx */
				break;
			}
		}
		count--;
	}

	*ptr = p;
	return p;

 fail:
	DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "regexp advance failed");
	return NULL;  /* never here */
}

/*
 *  Helpers for dealing with the input string
 */

/* Get a (possibly canonicalized) input character from current sp.
 * Note that the input itself is never modified, and captures
 * always record non-canonicalized strings even in case-insensitive
 * matching.
 */
static duk_u32 inp_get_u32(duk_re_matcher_ctx *re_ctx, duk_u8 **sp) {
	duk_u32 res = duk_unicode_xutf8_get_u32(re_ctx->thr, sp, re_ctx->input, re_ctx->input_end);
	if (re_ctx->re_flags & DUK_RE_FLAG_IGNORE_CASE) {
		res = duk_unicode_re_canonicalize_char(re_ctx->thr, res);
	}
	return res;
}

static duk_u8 *inp_backtrack(duk_re_matcher_ctx *re_ctx, duk_u8 **sp, duk_u32 count) {
	return utf8_backtrack(re_ctx->thr, sp, re_ctx->input, re_ctx->input_end, count);
}

/* Backtrack utf-8 input and return a (possibly canonicalized) input character. */
static int inp_getprev(duk_re_matcher_ctx *re_ctx, duk_u8 *sp) {
	/* note: caller 'sp' is intentionally not updated here */
	(void) inp_backtrack(re_ctx, &sp, 1);
	return inp_get_u32(re_ctx, &sp);
}
	
/*
 *  Regexp recursive matching function.
 *
 *  Returns 'sp' on successful match (points to character after last matched one),
 *  NULL otherwise.
 *
 *  The C recursion depth limit check is only performed in this function, this
 *  suffices because the function is present in all true recursion required by
 *  regexp execution.
 */

static duk_u8 *match_regexp(duk_re_matcher_ctx *re_ctx, duk_u8 *pc, duk_u8 *sp) {
	if (re_ctx->recursion >= re_ctx->recursion_limit) {
		DUK_ERROR(re_ctx->thr, DUK_ERR_INTERNAL_ERROR, "regexp recursion limit reached");
	}
	re_ctx->recursion++;

	for (;;) {
		int op;

		if (re_ctx->steps >= re_ctx->steps_limit) {
			DUK_ERROR(re_ctx->thr, DUK_ERR_INTERNAL_ERROR, "regexp step limit reached");
		}
		re_ctx->steps++;

		op = bc_get_u32(re_ctx, &pc);

		DUK_DDDPRINT("match: rec=%d, steps=%d, pc (after op)=%d, sp=%d, op=%d",
		             re_ctx->recursion,
		             re_ctx->steps,
		             (int) (pc - re_ctx->bytecode),
		             (int) (sp - re_ctx->input),
		             op);

		switch (op) {
		case DUK_REOP_MATCH: {
			goto match;
		}
		case DUK_REOP_CHAR: {
			/*
			 *  Byte-based matching would be possible for case-sensitive
			 *  matching but not for case-insensitive matching, but we
			 *  match by decoding the input and bytecode character normally.
			 *
			 *  Bytecode characters are assumed to be already canonicalized.
			 *  Input characters are canonicalized automatically by
			 *  inp_get_u32() if necessary.
			 *
			 *  There is no opcode for matching multiple characters.  The
			 *  regexp compiler has trouble joining strings efficiently
			 *  during compilation.  See doc/regular-expressions.txt for
			 *  more discussion.
			 */
			unsigned int c1, c2;

			c1 = bc_get_u32(re_ctx, &pc);
			DUK_ASSERT(!(re_ctx->re_flags & DUK_RE_FLAG_IGNORE_CASE) ||
			           c1 == duk_unicode_re_canonicalize_char(re_ctx->thr, c1));  /* canonicalized by compiler */
			if (sp >= re_ctx->input_end) {
				goto fail;
			}
			c2 = inp_get_u32(re_ctx, &sp);
			DUK_DDDPRINT("char match, c1=%d, c2=%d", c1, c2);
			if (c1 != c2) {
				goto fail;
			}
			break;
		}
		case DUK_REOP_PERIOD: {
			unsigned int c;

			if (sp >= re_ctx->input_end) {
				goto fail;
			}
			c = inp_get_u32(re_ctx, &sp);
			if (duk_unicode_is_line_terminator(c)) {
				/* E5 Sections 15.10.2.8, 7.3 */
				goto fail;
			}
			break;
		}
		case DUK_REOP_RANGES:
		case DUK_REOP_INVRANGES: {
			unsigned int n;
			unsigned int c;
			int match;
	
			n = bc_get_u32(re_ctx, &pc);
			if (sp >= re_ctx->input_end) {
				goto fail;
			}
			c = inp_get_u32(re_ctx, &sp);

			match = 0;
			while (n) {
				unsigned int r1, r2;
				r1 = bc_get_u32(re_ctx, &pc);
				r2 = bc_get_u32(re_ctx, &pc);
				DUK_DDDPRINT("matching ranges/invranges, n=%d, r1=%d, r2=%d, c=%d",
				             n, r1, r2, c);
				if (c >= r1 && c <= r2) {
					/* Note: don't bail out early, we must read all the ranges from
					 * bytecode.  Another option is to skip them efficiently after
					 * breaking out of here.  Prefer smallest code.
					 */
					match = 1;
				}
				n--;
			}

			if (op == DUK_REOP_RANGES) {
				if (!match) {
					goto fail;
				}
			} else {
				/* op == DUK_REOP_INVRANGES */
				if (match) {
					goto fail;
				}
			}
			break;
		}
		case DUK_REOP_ASSERT_START: {
			unsigned int c;

			if (sp <= re_ctx->input) {
				break;
			}
			if (!(re_ctx->re_flags & DUK_RE_FLAG_MULTILINE)) {
				goto fail;
			}
			c = inp_getprev(re_ctx, sp);
			if (duk_unicode_is_line_terminator(c)) {
				/* E5 Sections 15.10.2.8, 7.3 */
				break;
			}
			goto fail;
		}
		case DUK_REOP_ASSERT_END: {
			unsigned int c;
			duk_u8 *temp_sp;

			if (sp >= re_ctx->input_end) {
				break;
			}
			if (!(re_ctx->re_flags & DUK_RE_FLAG_MULTILINE)) {
				goto fail;
			}
			temp_sp = sp;
			c = inp_get_u32(re_ctx, &temp_sp);
			if (duk_unicode_is_line_terminator(c)) {
				/* E5 Sections 15.10.2.8, 7.3 */
				break;
			}
			goto fail;
		}
		case DUK_REOP_ASSERT_WORD_BOUNDARY:
		case DUK_REOP_ASSERT_NOT_WORD_BOUNDARY: {
			/*
			 *  E5 Section 15.10.2.6.  The previous and current character
			 *  should -not- be canonicalized as they are now.  However,
			 *  canonicalization does not affect the result of IsWordChar()
			 *  (which depends on Unicode characters never canonicalizing
			 *  into ASCII characters) so this does not matter.
			 */
			int c1, c2;  /* Note: negative value used as 'out of bounds' marker */

			if (sp <= re_ctx->input) {
				c1 = -1;
			} else {
				c1 = inp_getprev(re_ctx, sp);
			}
			c1 = duk_unicode_re_is_wordchar(c1);
			if (sp >= re_ctx->input_end) {
				c2 = -1;
			} else {
				duk_u8 *tmp_sp = sp;
				c2 = inp_get_u32(re_ctx, &tmp_sp);
			}
			c2 = duk_unicode_re_is_wordchar(c2);

			if (op == DUK_REOP_ASSERT_WORD_BOUNDARY) {
				if (c1 == c2) {
					goto fail;
				}
			} else {
				DUK_ASSERT(op == DUK_REOP_ASSERT_NOT_WORD_BOUNDARY);
				if (c1 != c2) {
					goto fail;
				}
			}
			break;
		}
		case DUK_REOP_JUMP: {
			duk_i32 skip;

			skip = bc_get_i32(re_ctx, &pc);
			pc += skip;
			break;
		}
		case DUK_REOP_SPLIT1: {
			/* split1: prefer direct execution (no jump) */
			duk_u8 *sub_sp;
			duk_i32 skip;

			skip = bc_get_i32(re_ctx, &pc);
			sub_sp = match_regexp(re_ctx, pc, sp);
			if (sub_sp) {
				sp = sub_sp;
				goto match;
			}
			pc += skip;
			break;
		}
		case DUK_REOP_SPLIT2: {
			/* split2: prefer jump execution (not direct) */
			duk_u8 *sub_sp;
			duk_i32 skip;

			skip = bc_get_i32(re_ctx, &pc);
			sub_sp = match_regexp(re_ctx, pc + skip, sp);
			if (sub_sp) {
				sp = sub_sp;
				goto match;
			}
			break;
		}
		case DUK_REOP_SQMINIMAL: {
			duk_u32 q, qmin, qmax;
			duk_i32 skip;
			duk_u8 *sub_sp;

			qmin = bc_get_u32(re_ctx, &pc);
			qmax = bc_get_u32(re_ctx, &pc);
			skip = bc_get_i32(re_ctx, &pc);
			DUK_DDDPRINT("minimal quantifier, qmin=%u, qmax=%u, skip=%d",
			             (unsigned int) qmin, (unsigned int) qmax, (int) skip);

			q = 0;
			while (q <= qmax) {
				if (q >= qmin) {
					sub_sp = match_regexp(re_ctx, pc + skip, sp);
					if (sub_sp) {
						sp = sub_sp;
						goto match;
					}
				}
				sub_sp = match_regexp(re_ctx, pc, sp);
				if (!sub_sp) {
					break;
				}
				sp = sub_sp;
				q++;
			}
			goto fail;
		}
		case DUK_REOP_SQGREEDY: {
			duk_u32 q, qmin, qmax, atomlen;
			duk_i32 skip;
			duk_u8 *sub_sp;

			qmin = bc_get_u32(re_ctx, &pc);
			qmax = bc_get_u32(re_ctx, &pc);
			atomlen = bc_get_u32(re_ctx, &pc);
			skip = bc_get_i32(re_ctx, &pc);
			DUK_DDDPRINT("greedy quantifier, qmin=%u, qmax=%u, atomlen=%u, skip=%d",
			             (unsigned int) qmin, (unsigned int) qmax, (unsigned int) atomlen, (int) skip);

			q = 0;
			while (q < qmax) {
				sub_sp = match_regexp(re_ctx, pc, sp);
				if (!sub_sp) {
					break;
				}
				sp = sub_sp;
				q++;
			}
			while (q >= qmin) {
				sub_sp = match_regexp(re_ctx, pc + skip, sp);
				if (sub_sp) {
					sp = sub_sp;
					goto match;
				}
				if (q == qmin) {
					break;
				}

				/* Note: if atom were to contain e.g. captures, we would need to
				 * re-match the atom to get correct captures.  Simply quantifiers
				 * do not allow captures in their atom now, so this is not an issue.
				 */

				DUK_DDDPRINT("greedy quantifier, backtrack %d characters (atomlen)",
				             atomlen);
				sp = inp_backtrack(re_ctx, &sp, atomlen);
				q--;
			}
			goto fail;
		}
		case DUK_REOP_SAVE: {
			duk_u32 idx;
			duk_u8 *old;
			duk_u8 *sub_sp;

			idx = bc_get_u32(re_ctx, &pc);
			if (idx >= re_ctx->nsaved) {
				/* idx is unsigned, < 0 check is not necessary */
				DUK_DPRINT("internal error, regexp save index insane");
				goto internal_error;
			}
			old = re_ctx->saved[idx];
			re_ctx->saved[idx] = sp;
			sub_sp = match_regexp(re_ctx, pc, sp);
			if (sub_sp) {
				sp = sub_sp;
				goto match;
			}
			re_ctx->saved[idx] = old;
			goto fail;
		}
		case DUK_REOP_LOOKPOS:
		case DUK_REOP_LOOKNEG: {
			/*
			 *  Needs a save of multiple saved[] entries depending on what range
			 *  may be overwritten.  Because the regexp parser does no such analysis,
			 *  we currently save the entire saved array here.  Lookaheads are thus
			 *  a bit expensive.  Note that the saved array is not needed for just
			 *  the lookahead sub-match, but for the matching of the entire sequel.
			 *
			 *  The temporary save buffer is pushed on to the valstack to handle
			 *  errors correctly.  Each lookahead causes a C recursion and pushes
			 *  more stuff on the value stack.  If the C recursion limit is less
			 *  than the value stack spare, there is no need to check the stack.
			 *  We do so regardless, just in case.
			 */

			duk_i32 skip;
			duk_u8 **full_save;
			duk_u8 *sub_sp;

			DUK_ASSERT(re_ctx->nsaved > 0);

			duk_require_stack((duk_context *) re_ctx->thr, 1);
			full_save = duk_push_new_fixed_buffer((duk_context *) re_ctx->thr,
			                                      sizeof(duk_u8 *) * re_ctx->nsaved);
			DUK_ASSERT(full_save != NULL);
			memcpy(full_save, re_ctx->saved, sizeof(duk_u8 *) * re_ctx->nsaved);

			skip = bc_get_i32(re_ctx, &pc);
			sub_sp = match_regexp(re_ctx, pc, sp);
			if (op == DUK_REOP_LOOKPOS) {
				if (!sub_sp) {
					goto lookahead_fail;
				}
			} else {
				if (sub_sp) {
					goto lookahead_fail;
				}
			}
			sub_sp = match_regexp(re_ctx, pc + skip, sp);
			if (sub_sp) {
				/* match: keep saves */
				duk_pop((duk_context *) re_ctx->thr);
				sp = sub_sp;
				goto match;
			}

			/* fall through */

		 lookahead_fail:
			/* fail: restore saves */
			memcpy(re_ctx->saved, full_save, sizeof(duk_u8 *) * re_ctx->nsaved);
			duk_pop((duk_context *) re_ctx->thr);
			goto fail;
		}
		case DUK_REOP_BACKREFERENCE: {
			/*
			 *  Byte matching for back-references would be OK in case-
			 *  sensitive matching.  In case-insensitive matching we need
			 *  to canonicalize characters, so back-reference matching needs
			 *  to be done with codepoints instead.  So, we just decode
			 *  everything normally here, too.
			 *
			 *  Note: back-reference index which is 0 or higher than
			 *  NCapturingParens (= number of capturing parens in the
			 *  -entire- regexp) is a compile time error.  However, a
			 *  backreference referring to a valid capture which has
			 *  not matched anything always succeeds!  See E5 Section
			 *  15.10.2.9, step 5, sub-step 3.
			 */
			duk_u32 idx;
			duk_u8 *p;

			idx = bc_get_u32(re_ctx, &pc);
			idx = idx << 1;		/* backref n -> saved indices [n*2, n*2+1] */
			if (idx < 2 || idx + 1 >= re_ctx->nsaved) {
				/* regexp compiler should catch these */
				DUK_DPRINT("internal error, backreference index insane");
				goto internal_error;
			}
			if (!re_ctx->saved[idx] || !re_ctx->saved[idx+1]) {
				/* capture is 'undefined', always matches! */
				DUK_DDDPRINT("backreference: saved[%d,%d] not complete, always match",
				             idx, idx+1);
				break;
			}
			DUK_DDDPRINT("backreference: match saved[%d,%d]", idx, idx+1);

			p = re_ctx->saved[idx];
			while (p < re_ctx->saved[idx+1]) {
				unsigned int c1, c2;

				/* Note: not necessary to check p against re_ctx->input_end:
				 * the memory access is checked by inp_get_u32(), while
				 * valid compiled regexps cannot write a saved[] entry
				 * which points to outside the string.
				 */
				if (sp >= re_ctx->input_end) {
					goto fail;
				}
				c1 = inp_get_u32(re_ctx, &p);
				c2 = inp_get_u32(re_ctx, &sp);
				if (c1 != c2) {
					goto fail;
				}
			}
			break;
		}
		default: {
			DUK_DPRINT("internal error, regexp opcode error: %d", op);
			goto internal_error;
		}
		}
	}

 match:
	re_ctx->recursion--;
	return sp;

 fail:
	re_ctx->recursion--;
	return NULL;

 internal_error:
	DUK_ERROR(re_ctx->thr, DUK_ERR_INTERNAL_ERROR, "regexp internal error");
	return NULL;  /* never here */
}

/*
 *  Exposed matcher function which provides the semantics of RegExp.prototype.exec().
 *
 *  RegExp.prototype.test() has the same semantics as exec() but does not return the
 *  result object (which contains the matching string and capture groups).  Currently
 *  there is no separate test() helper, so a temporary result object is created and
 *  discarded if test() is needed.  This is intentional, to save code space.
 *
 *  Input stack:  [ ... re_obj input ]
 *  Output stack: [ ... result ]
 */

void duk_regexp_match(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_re_matcher_ctx re_ctx;
	duk_hobject *h_regexp;
	duk_hstring *h_bytecode;
	duk_hstring *h_input;
	duk_u8 *pc;
	duk_u8 *sp;
	int match = 0;
	int i;
	double d;
	duk_u32 char_offset;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);

	DUK_DDPRINT("regexp match: regexp=%!T, input=%!T", duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));

	/*
	 *  Regexp instance check, bytecode check, input coercion.
	 *
	 *  See E5 Section 15.10.6.
	 */

	/* TypeError if wrong; class check, see E5 Section 15.10.6 */
	h_regexp = duk_require_hobject_with_class(ctx, -2, DUK_HOBJECT_CLASS_REGEXP);
	DUK_ASSERT(h_regexp != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_CLASS_NUMBER(h_regexp) == DUK_HOBJECT_CLASS_REGEXP);
	h_regexp = h_regexp;  /* suppress warning */

	duk_to_string(ctx, -1);
	h_input = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h_input != NULL);

	duk_get_prop_stridx(ctx, -2, DUK_HEAP_STRIDX_INT_BYTECODE);  /* [ ... re_obj input ] -> [ ... re_obj input bc ] */
	h_bytecode = duk_require_hstring(ctx, -1);  /* no regexp instance should exist without a non-configurable bytecode property */
	DUK_ASSERT(h_bytecode != NULL);

	/*
	 *  Basic context initialization.
	 *
	 *  Some init values are read from the bytecode header
	 *  whose format is (UTF-8 codepoints):
	 *
	 *    uint   flags
	 *    uint   nsaved (even, 2n+2 where n = num captures)
	 */

	/* [ ... re_obj input bc ] */

	memset(&re_ctx, 0, sizeof(re_ctx));

	re_ctx.thr = thr;
	re_ctx.input = (duk_u8 *) DUK_HSTRING_GET_DATA(h_input);
	re_ctx.input_end = re_ctx.input + DUK_HSTRING_GET_BYTELEN(h_input);
	re_ctx.bytecode = (duk_u8 *) DUK_HSTRING_GET_DATA(h_bytecode);
	re_ctx.bytecode_end = re_ctx.bytecode + DUK_HSTRING_GET_BYTELEN(h_bytecode);
	re_ctx.saved = NULL;
	re_ctx.recursion_limit = DUK_RE_EXECUTE_RECURSION_LIMIT;
	re_ctx.steps_limit = DUK_RE_EXECUTE_STEPS_LIMIT;

	/* read header */
	pc = re_ctx.bytecode;
	re_ctx.re_flags = bc_get_u32(&re_ctx, &pc);
	re_ctx.nsaved = bc_get_u32(&re_ctx, &pc);
	re_ctx.bytecode = pc;

	DUK_ASSERT(re_ctx.nsaved >= 2);
	DUK_ASSERT((re_ctx.nsaved % 2) == 0);

	duk_push_new_fixed_buffer(ctx, sizeof(duk_u8 *) * re_ctx.nsaved);
	re_ctx.saved = duk_get_buffer(ctx, -1, NULL);
	DUK_ASSERT(re_ctx.saved != NULL);

	/* [ ... re_obj input bc saved_buf ] */

	/* buffer is automatically zeroed */
#ifdef DUK_USE_EXPLICIT_NULL_INIT
	for (i = 0; i < re_ctx.nsaved; i++) {
		re_ctx.saved[i] = (duk_u8 *) NULL;
	}
#endif

	DUK_DDDPRINT("regexp ctx initialized, flags=0x%08x, nsaved=%d, recursion_limit=%d, steps_limit=%d",
	             re_ctx.re_flags, re_ctx.nsaved, re_ctx.recursion_limit, re_ctx.steps_limit);

	/*
	 *  Get starting character offset for match, and initialize 'sp' based on it.
	 *
	 *  Note: lastIndex is non-configurable so it must be present (we check the
	 *  internal class of the object above, so we know it is).  User code can set
	 *  its value to an arbitrary (garbage) value though; E5 requires that lastIndex
	 *  be coerced to a number before using.  The code below works even if the
	 *  property is missing: the value will then be coerced to zero.
	 *
	 *  Note: lastIndex may be outside Uint32 range even after ToInteger() coercion.
	 *  For instance, ToInteger(+Infinity) = +Infinity.  We track the match offset
	 *  as an integer, but pre-check it to be inside the 32-bit range before the loop.
	 *  If not, the check in E5 Section 15.10.6.2, step 9.a applies.
	 */

	/* XXX: lastIndex handling produces a lot of asm */

	/* [ ... re_obj input bc saved_buf ] */

	duk_get_prop_stridx(ctx, -4, DUK_HEAP_STRIDX_LAST_INDEX);  /* -> [ ... re_obj input bc saved_buf lastIndex ] */
	(void) duk_to_int(ctx, -1);  /* ToInteger(lastIndex) */
	d = duk_get_number(ctx, -1);  /* integer, but may be +/- Infinite, +/- zero (not NaN, though) */
	duk_pop(ctx);

	if (re_ctx.re_flags & DUK_RE_FLAG_GLOBAL) {
		if (d < 0.0 || d > (double) DUK_HSTRING_GET_CHARLEN(h_input)) {
			/* match fail */
			char_offset = 0;   /* not really necessary */
			DUK_ASSERT(match == 0);
			goto match_over;
		}
		char_offset = (duk_u32) d;
	} else {
		/* lastIndex must be ignored for non-global regexps, but get the
		 * value for (theoretical) side effects.  No side effects can
		 * really occur, because lastIndex is a normal property and is
		 * always non-configurable for RegExp instances.
		 */
		char_offset = (duk_u32) 0;
	}

	sp = re_ctx.input + duk_heap_strcache_offset_char2byte(thr, h_input, char_offset);

	/*
	 *  Match loop.
	 *
	 *  Try matching at different offsets until match found or input exhausted.
	 */

	/* [ ... re_obj input bc saved_buf ] */

	DUK_ASSERT(match == 0);

	for (;;) {
		/* char offset in [0, h_input->clen] (both ends inclusive), checked before entry */
		DUK_ASSERT(char_offset >= 0 && char_offset <= DUK_HSTRING_GET_CHARLEN(h_input));

		/* Note: ctx.steps is intentionally not reset, it applies to the entire unanchored match */
		DUK_ASSERT(re_ctx.recursion == 0);

		DUK_DDDPRINT("attempt match at char offset %d; %p [%p,%p]",
		             char_offset, sp, re_ctx.input, re_ctx.input_end);

		/*
		 *  Note:
		 *
		 *    - match_regexp() is required not to longjmp() in ordinary "non-match"
		 *      conditions; a longjmp() will terminate the entire matching process.
		 *
		 *    - Clearing saved[] is not necessary because backtracking does it
		 *
		 *    - Backtracking also rewinds ctx.recursion back to zero, unless an
		 *      internal/limit error occurs (which causes a longjmp())
		 *
		 *    - If we supported anchored matches, we would break out here
		 *      unconditionally; however, Ecmascript regexps don't have anchored
		 *      matches.  It might make sense to implement a fast bail-out if
		 *      the regexp begins with '^' and sp is not 0: currently we'll just
		 *      run through the entire input string, trivially failing the match
		 *      at every non-zero offset.
		 */

		if (match_regexp(&re_ctx, re_ctx.bytecode, sp) != NULL) {
			DUK_DDDPRINT("match at offset %d", char_offset);
			match = 1;
			break;
		}

		/* advance by one character (code point) and one char_offset */
		char_offset++;
		if (char_offset > (double) DUK_HSTRING_GET_CHARLEN(h_input)) {
			/*
			 *  Note:
			 *
			 *    - Intentionally attempt (empty) match at char_offset == k_input->clen
			 *
			 *    - Negative char_offsets have been eliminated and char_offset is duk_u32
			 *      -> no need or use for a negative check
			 */

			DUK_DDDPRINT("no match after trying all sp offsets");
			break;
		}

		(void) utf8_advance(thr, &sp, re_ctx.input, re_ctx.input_end, 1);  /* avoid calling at end of input, will DUK_ERROR (above check suffices) */
	}

 match_over:

	/*
	 *  Matching complete, create result array or return a 'null'.  Update lastIndex
	 *  if necessary.  See E5 Section 15.10.6.2.
	 *
	 *  Because lastIndex is a character (not byte) offset, we need the character
	 *  length of the match which we conveniently get as a side effect of interning
	 *  the matching substring (0th index of result array).
	 *
	 *  saved[0]         start pointer (~ byte offset) of current match
	 *  saved[1]         end pointer (~ byte offset) of current match (exclusive)
	 *  char_offset      start character offset of current match (-> .index of result)
	 *  char_end_offset  end character offset (computed below)
	 */

	/* [ ... re_obj input bc saved_buf ] */

	if (match) {
#ifdef DUK_USE_ASSERTIONS
		duk_hobject *h_res;
#endif
		duk_u32 char_end_offset = 0;

		DUK_DDDPRINT("regexp matches at char_offset %d", (int) char_offset);

		DUK_ASSERT(re_ctx.nsaved >= 2);        /* must have start and end */
		DUK_ASSERT((re_ctx.nsaved % 2) == 0);  /* and even number */

		/* XXX: Array size is known before and (2 * re_ctx.nsaved) but not taken advantage
		 * of now.  The array is not compacted either, as regexp match objects are usually
		 * short lived.
		 */

		duk_push_new_array(ctx);

#ifdef DUK_USE_ASSERTIONS
		h_res = duk_require_hobject(ctx, -1);
		DUK_ASSERT(DUK_HOBJECT_HAS_EXTENSIBLE(h_res));
		DUK_ASSERT(DUK_HOBJECT_HAS_SPECIAL_ARRAY(h_res));
		DUK_ASSERT(DUK_HOBJECT_GET_CLASS_NUMBER(h_res) == DUK_HOBJECT_CLASS_ARRAY);
#endif

		/* [ ... re_obj input bc saved_buf res_obj ] */

		duk_push_number(ctx, (double) char_offset);
		duk_def_prop_stridx(ctx, -2, DUK_HEAP_STRIDX_INDEX, DUK_PROPDESC_FLAGS_WEC);

		duk_dup(ctx, -4);
		duk_def_prop_stridx(ctx, -2, DUK_HEAP_STRIDX_INPUT, DUK_PROPDESC_FLAGS_WEC);

		for (i = 0; i < re_ctx.nsaved; i += 2) {
			/* If saved[] pointers are insane, we just ignore them instead of
			 * throwing an internal error; the captures are returned as 'undefined'.
			 * This should, of course, never happen in practice.
			 */
			if (re_ctx.saved[i] && re_ctx.saved[i+1] && re_ctx.saved[i+1] >= re_ctx.saved[i]) {
				duk_hstring *h_saved;

				duk_push_lstring(ctx,
				                 (char *) re_ctx.saved[i],
				                 (size_t) (re_ctx.saved[i+1] - re_ctx.saved[i]));
				h_saved = duk_get_hstring(ctx, -1);
				DUK_ASSERT(h_saved != NULL);

				if (i == 0) {
					/* Assumes that saved[0] and saved[1] are always
					 * set by regexp bytecode (if not, char_end_offset
					 * will be zero).  Also assumes clen reflects the
					 * correct char length.
					 */
					char_end_offset = char_offset + DUK_HSTRING_GET_CHARLEN(h_saved);
				}
			} else {
				duk_push_undefined(ctx);
			}

			/* [ ... re_obj input bc saved_buf res_obj val ] */
			duk_put_prop_index(ctx, -2, i / 2);
		}

		/* [ ... re_obj input bc saved_buf res_obj ] */

		/* NB: 'length' property is automatically updated by the array setup loop */

		if (re_ctx.re_flags & DUK_RE_FLAG_GLOBAL) {
			/* global regexp: lastIndex updated on match */
			duk_push_number(ctx, (double) char_end_offset);
			duk_put_prop_stridx(ctx, -6, DUK_HEAP_STRIDX_LAST_INDEX);
		} else {
			/* non-global regexp: lastIndex never updated on match */
			;
		}
	} else {
		/*
		 *  No match, E5 Section 15.10.6.2, step 9.a.i - 9.a.ii apply, regardless
		 *  of 'global' flag of the RegExp.  In particular, if lastIndex is invalid
		 *  initially, it is reset to zero.
		 */

		DUK_DDDPRINT("regexp does not match");

		duk_push_null(ctx);

		/* [ ... re_obj input bc saved_buf res_obj ] */

		duk_push_int(ctx, 0);
		duk_put_prop_stridx(ctx, -6, DUK_HEAP_STRIDX_LAST_INDEX);
	}

	/* [ ... re_obj input bc saved_buf res_obj ] */

	duk_insert(ctx, -5);

	/* [ ... res_obj re_obj input bc saved_buf ] */

	duk_pop_n(ctx, 4);

	/* [ ... res_obj ] */

	/* XXX: these last tricks are unnecessary if the function is made
	 * a genuine native function.
	 */
}

#endif  /* DUK_USE_REGEXP_SUPPORT */

