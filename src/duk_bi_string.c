/*
 *  String built-ins
 */

/* XXX: There are several limitations in the current implementation for
 * strings with >= 0x80000000UL characters.  In some cases one would need
 * to be able to represent the range [-0xffffffff,0xffffffff] and so on.
 * Generally character and byte length are assumed to fit into signed 32
 * bits (< 0x80000000UL).  Places with issues are not marked explicitly
 * below in all cases, look for signed type usage (duk_int_t etc) for
 * offsets/lengths.
 */

#include "duk_internal.h"

/*
 *  Constructor
 */

DUK_INTERNAL duk_ret_t duk_bi_string_constructor(duk_context *ctx) {
	/* String constructor needs to distinguish between an argument not given at all
	 * vs. given as 'undefined'.  We're a vararg function to handle this properly.
	 */

	if (duk_get_top(ctx) == 0) {
		duk_push_hstring_stridx(ctx, DUK_STRIDX_EMPTY_STRING);
	} else {
		duk_to_string(ctx, 0);
	}
	DUK_ASSERT(duk_is_string(ctx, 0));
	duk_set_top(ctx, 1);

	if (duk_is_constructor_call(ctx)) {
		duk_push_object_helper(ctx,
		                       DUK_HOBJECT_FLAG_EXTENSIBLE |
		                       DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ |
		                       DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_STRING),
		                       DUK_BIDX_STRING_PROTOTYPE);

		/* String object internal value is immutable */
		duk_dup(ctx, 0);
		duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);
	}
	/* Note: unbalanced stack on purpose */

	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_string_constructor_from_char_code(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_bufwriter_ctx bw_alloc;
	duk_bufwriter_ctx *bw;
	duk_idx_t i, n;
	duk_ucodepoint_t cp;

	/* XXX: It would be nice to build the string directly but ToUint16()
	 * coercion is needed so a generic helper would not be very
	 * helpful (perhaps coerce the value stack first here and then
	 * build a string from a duk_tval number sequence in one go?).
	 */

	n = duk_get_top(ctx);

	bw = &bw_alloc;
	DUK_BW_INIT_PUSHBUF(thr, bw, n);  /* initial estimate for ASCII only codepoints */

	for (i = 0; i < n; i++) {
		/* XXX: could improve bufwriter handling to write multiple codepoints
		 * with one ensure call but the relative benefit would be quite small.
		 */

#if defined(DUK_USE_NONSTD_STRING_FROMCHARCODE_32BIT)
		/* ToUint16() coercion is mandatory in the E5.1 specification, but
		 * this non-compliant behavior makes more sense because we support
		 * non-BMP codepoints.  Don't use CESU-8 because that'd create
		 * surrogate pairs.
		 */

		cp = (duk_ucodepoint_t) duk_to_uint32(ctx, i);
		DUK_BW_WRITE_ENSURE_XUTF8(thr, bw, cp);
#else
		cp = (duk_ucodepoint_t) duk_to_uint16(ctx, i);
		DUK_BW_WRITE_ENSURE_CESU8(thr, bw, cp);
#endif
	}

	DUK_BW_COMPACT(thr, bw);
	duk_to_string(ctx, -1);
	return 1;
}

/*
 *  toString(), valueOf()
 */

DUK_INTERNAL duk_ret_t duk_bi_string_prototype_to_string(duk_context *ctx) {
	duk_tval *tv;

	duk_push_this(ctx);
	tv = duk_require_tval(ctx, -1);
	DUK_ASSERT(tv != NULL);

	if (DUK_TVAL_IS_STRING(tv)) {
		/* return as is */
		return 1;
	} else if (DUK_TVAL_IS_OBJECT(tv)) {
		duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);

		/* Must be a "string object", i.e. class "String" */
		if (DUK_HOBJECT_GET_CLASS_NUMBER(h) != DUK_HOBJECT_CLASS_STRING) {
			goto type_error;
		}

		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VALUE);
		DUK_ASSERT(duk_is_string(ctx, -1));

		return 1;
	} else {
		goto type_error;
	}

	/* never here, but fall through */

 type_error:
	return DUK_RET_TYPE_ERROR;
}

/*
 *  Character and charcode access
 */

DUK_INTERNAL duk_ret_t duk_bi_string_prototype_char_at(duk_context *ctx) {
	duk_int_t pos;

	/* XXX: faster implementation */

	(void) duk_push_this_coercible_to_string(ctx);
	pos = duk_to_int(ctx, 0);
	duk_substring(ctx, -1, pos, pos + 1);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_string_prototype_char_code_at(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_int_t pos;
	duk_hstring *h;
	duk_bool_t clamped;

	/* XXX: faster implementation */

	DUK_DDD(DUK_DDDPRINT("arg=%!T", (duk_tval *) duk_get_tval(ctx, 0)));

	h = duk_push_this_coercible_to_string(ctx);
	DUK_ASSERT(h != NULL);

	pos = duk_to_int_clamped_raw(ctx,
	                             0 /*index*/,
	                             0 /*min(incl)*/,
	                             DUK_HSTRING_GET_CHARLEN(h) - 1 /*max(incl)*/,
	                             &clamped /*out_clamped*/);
	if (clamped) {
		duk_push_number(ctx, DUK_DOUBLE_NAN);
		return 1;
	}

	duk_push_u32(ctx, (duk_uint32_t) duk_hstring_char_code_at_raw(thr, h, pos));
	return 1;
}

/*
 *  substring(), substr(), slice()
 */

/* XXX: any chance of merging these three similar but still slightly
 * different algorithms so that footprint would be reduced?
 */

DUK_INTERNAL duk_ret_t duk_bi_string_prototype_substring(duk_context *ctx) {
	duk_hstring *h;
	duk_int_t start_pos, end_pos;
	duk_int_t len;

	h = duk_push_this_coercible_to_string(ctx);
	DUK_ASSERT(h != NULL);
	len = (duk_int_t) DUK_HSTRING_GET_CHARLEN(h);

	/* [ start end str ] */

	start_pos = duk_to_int_clamped(ctx, 0, 0, len);
	if (duk_is_undefined(ctx, 1)) {
		end_pos = len;
	} else {
		end_pos = duk_to_int_clamped(ctx, 1, 0, len);
	}
	DUK_ASSERT(start_pos >= 0 && start_pos <= len);
	DUK_ASSERT(end_pos >= 0 && end_pos <= len);

	if (start_pos > end_pos) {
		duk_int_t tmp = start_pos;
		start_pos = end_pos;
		end_pos = tmp;
	}

	DUK_ASSERT(end_pos >= start_pos);

	duk_substring(ctx, -1, (duk_size_t) start_pos, (duk_size_t) end_pos);
	return 1;
}

#ifdef DUK_USE_SECTION_B
DUK_INTERNAL duk_ret_t duk_bi_string_prototype_substr(duk_context *ctx) {
	duk_hstring *h;
	duk_int_t start_pos, end_pos;
	duk_int_t len;

	/* Unlike non-obsolete String calls, substr() algorithm in E5.1
	 * specification will happily coerce undefined and null to strings
	 * ("undefined" and "null").
	 */
	duk_push_this(ctx);
	h = duk_to_hstring(ctx, -1);
	DUK_ASSERT(h != NULL);
	len = (duk_int_t) DUK_HSTRING_GET_CHARLEN(h);

	/* [ start length str ] */

	/* The implementation for computing of start_pos and end_pos differs
	 * from the standard algorithm, but is intended to result in the exactly
	 * same behavior.  This is not always obvious.
	 */

	/* combines steps 2 and 5; -len ensures max() not needed for step 5 */
	start_pos = duk_to_int_clamped(ctx, 0, -len, len);
	if (start_pos < 0) {
		start_pos = len + start_pos;
	}
	DUK_ASSERT(start_pos >= 0 && start_pos <= len);

	/* combines steps 3, 6; step 7 is not needed */
	if (duk_is_undefined(ctx, 1)) {
		end_pos = len;
	} else {
		DUK_ASSERT(start_pos <= len);
		end_pos = start_pos + duk_to_int_clamped(ctx, 1, 0, len - start_pos);
	}
	DUK_ASSERT(start_pos >= 0 && start_pos <= len);
	DUK_ASSERT(end_pos >= 0 && end_pos <= len);
	DUK_ASSERT(end_pos >= start_pos);

	duk_substring(ctx, -1, (duk_size_t) start_pos, (duk_size_t) end_pos);
	return 1;
}
#else  /* DUK_USE_SECTION_B */
DUK_INTERNAL duk_ret_t duk_bi_string_prototype_substr(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
#endif  /* DUK_USE_SECTION_B */

DUK_INTERNAL duk_ret_t duk_bi_string_prototype_slice(duk_context *ctx) {
	duk_hstring *h;
	duk_int_t start_pos, end_pos;
	duk_int_t len;

	h = duk_push_this_coercible_to_string(ctx);
	DUK_ASSERT(h != NULL);
	len = (duk_int_t) DUK_HSTRING_GET_CHARLEN(h);

	/* [ start end str ] */

	start_pos = duk_to_int_clamped(ctx, 0, -len, len);
	if (start_pos < 0) {
		start_pos = len + start_pos;
	}
	if (duk_is_undefined(ctx, 1)) {
		end_pos = len;
	} else {
		end_pos = duk_to_int_clamped(ctx, 1, -len, len);
		if (end_pos < 0) {
			end_pos = len + end_pos;
		}
	}
	DUK_ASSERT(start_pos >= 0 && start_pos <= len);
	DUK_ASSERT(end_pos >= 0 && end_pos <= len);

	if (end_pos < start_pos) {
		end_pos = start_pos;
	}

	DUK_ASSERT(end_pos >= start_pos);

	duk_substring(ctx, -1, (duk_size_t) start_pos, (duk_size_t) end_pos);
	return 1;
}

/*
 *  Case conversion
 */

DUK_INTERNAL duk_ret_t duk_bi_string_prototype_caseconv_shared(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_small_int_t uppercase = duk_get_current_magic(ctx);

	(void) duk_push_this_coercible_to_string(ctx);
	duk_unicode_case_convert_string(thr, (duk_bool_t) uppercase);
	return 1;
}

/*
 *  indexOf() and lastIndexOf()
 */

DUK_INTERNAL duk_ret_t duk_bi_string_prototype_indexof_shared(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_this;
	duk_hstring *h_search;
	duk_int_t clen_this;
	duk_int_t cpos;
	duk_int_t bpos;
	const duk_uint8_t *p_start, *p_end, *p;
	const duk_uint8_t *q_start;
	duk_int_t q_blen;
	duk_uint8_t firstbyte;
	duk_uint8_t t;
	duk_small_int_t is_lastindexof = duk_get_current_magic(ctx);  /* 0=indexOf, 1=lastIndexOf */

	h_this = duk_push_this_coercible_to_string(ctx);
	DUK_ASSERT(h_this != NULL);
	clen_this = (duk_int_t) DUK_HSTRING_GET_CHARLEN(h_this);

	h_search = duk_to_hstring(ctx, 0);
	DUK_ASSERT(h_search != NULL);
	q_start = DUK_HSTRING_GET_DATA(h_search);
	q_blen = (duk_int_t) DUK_HSTRING_GET_BYTELEN(h_search);

	duk_to_number(ctx, 1);
	if (duk_is_nan(ctx, 1) && is_lastindexof) {
		/* indexOf: NaN should cause pos to be zero.
		 * lastIndexOf: NaN should cause pos to be +Infinity
		 * (and later be clamped to len).
		 */
		cpos = clen_this;
	} else {
		cpos = duk_to_int_clamped(ctx, 1, 0, clen_this);
	}

	/* Empty searchstring always matches; cpos must be clamped here.
	 * (If q_blen were < 0 due to clamped coercion, it would also be
	 * caught here.)
	 */
	if (q_blen <= 0) {
		duk_push_int(ctx, cpos);
		return 1;
	}
	DUK_ASSERT(q_blen > 0);

	bpos = (duk_int_t) duk_heap_strcache_offset_char2byte(thr, h_this, (duk_uint32_t) cpos);

	p_start = DUK_HSTRING_GET_DATA(h_this);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_this);
	p = p_start + bpos;

	/* This loop is optimized for size.  For speed, there should be
	 * two separate loops, and we should ensure that memcmp() can be
	 * used without an extra "will searchstring fit" check.  Doing
	 * the preconditioning for 'p' and 'p_end' is easy but cpos
	 * must be updated if 'p' is wound back (backward scanning).
	 */

	firstbyte = q_start[0];  /* leading byte of match string */
	while (p <= p_end && p >= p_start) {
		t = *p;

		/* For Ecmascript strings, this check can only match for
		 * initial UTF-8 bytes (not continuation bytes).  For other
		 * strings all bets are off.
		 */

		if ((t == firstbyte) && ((duk_size_t) (p_end - p) >= (duk_size_t) q_blen)) {
			DUK_ASSERT(q_blen > 0);  /* no issues with memcmp() zero size, even if broken */
			if (DUK_MEMCMP((const void *) p, (const void *) q_start, (size_t) q_blen) == 0) {
				duk_push_int(ctx, cpos);
				return 1;
			}
		}

		/* track cpos while scanning */
		if (is_lastindexof) {
			/* when going backwards, we decrement cpos 'early';
			 * 'p' may point to a continuation byte of the char
			 * at offset 'cpos', but that's OK because we'll
			 * backtrack all the way to the initial byte.
			 */
			if ((t & 0xc0) != 0x80) {
				cpos--;
			}
			p--;
		} else {
			if ((t & 0xc0) != 0x80) {
				cpos++;
			}
			p++;
		}
	}

	/* Not found.  Empty string case is handled specially above. */
	duk_push_int(ctx, -1);
	return 1;
}

/*
 *  replace()
 */

/* XXX: the current implementation works but is quite clunky; it compiles
 * to almost 1,4kB of x86 code so it needs to be simplified (better approach,
 * shared helpers, etc).  Some ideas for refactoring:
 *
 * - a primitive to convert a string into a regexp matcher (reduces matching
 *   code at the cost of making matching much slower)
 * - use replace() as a basic helper for match() and split(), which are both
 *   much simpler
 * - API call to get_prop and to_boolean
 */

DUK_INTERNAL duk_ret_t duk_bi_string_prototype_replace(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_input;
	duk_hstring *h_match;
	duk_hstring *h_search;
	duk_hobject *h_re;
	duk_bufwriter_ctx bw_alloc;
	duk_bufwriter_ctx *bw;
#ifdef DUK_USE_REGEXP_SUPPORT
	duk_bool_t is_regexp;
	duk_bool_t is_global;
#endif
	duk_bool_t is_repl_func;
	duk_uint32_t match_start_coff, match_start_boff;
#ifdef DUK_USE_REGEXP_SUPPORT
	duk_int_t match_caps;
#endif
	duk_uint32_t prev_match_end_boff;
	const duk_uint8_t *r_start, *r_end, *r;   /* repl string scan */
	duk_size_t tmp_sz;

	DUK_ASSERT_TOP(ctx, 2);
	h_input = duk_push_this_coercible_to_string(ctx);
	DUK_ASSERT(h_input != NULL);

	bw = &bw_alloc;
	DUK_BW_INIT_PUSHBUF(thr, bw, DUK_HSTRING_GET_BYTELEN(h_input));  /* input size is good output starting point */

	DUK_ASSERT_TOP(ctx, 4);

	/* stack[0] = search value
	 * stack[1] = replace value
	 * stack[2] = input string
	 * stack[3] = result buffer
	 */

	h_re = duk_get_hobject_with_class(ctx, 0, DUK_HOBJECT_CLASS_REGEXP);
	if (h_re) {
#ifdef DUK_USE_REGEXP_SUPPORT
		is_regexp = 1;
		is_global = duk_get_prop_stridx_boolean(ctx, 0, DUK_STRIDX_GLOBAL, NULL);

		if (is_global) {
			/* start match from beginning */
			duk_push_int(ctx, 0);
			duk_put_prop_stridx(ctx, 0, DUK_STRIDX_LAST_INDEX);
		}
#else  /* DUK_USE_REGEXP_SUPPORT */
		return DUK_RET_UNSUPPORTED_ERROR;
#endif  /* DUK_USE_REGEXP_SUPPORT */
	} else {
		duk_to_string(ctx, 0);
#ifdef DUK_USE_REGEXP_SUPPORT
		is_regexp = 0;
		is_global = 0;
#endif
	}

	if (duk_is_function(ctx, 1)) {
		is_repl_func = 1;
		r_start = NULL;
		r_end = NULL;
	} else {
		duk_hstring *h_repl;

		is_repl_func = 0;
		h_repl = duk_to_hstring(ctx, 1);
		DUK_ASSERT(h_repl != NULL);
		r_start = DUK_HSTRING_GET_DATA(h_repl);
		r_end = r_start + DUK_HSTRING_GET_BYTELEN(h_repl);
	}

	prev_match_end_boff = 0;

	for (;;) {
		/*
		 *  If matching with a regexp:
		 *    - non-global RegExp: lastIndex not touched on a match, zeroed
		 *      on a non-match
		 *    - global RegExp: on match, lastIndex will be updated by regexp
		 *      executor to point to next char after the matching part (so that
		 *      characters in the matching part are not matched again)
		 *
		 *  If matching with a string:
		 *    - always non-global match, find first occurrence
		 *
		 *  We need:
		 *    - The character offset of start-of-match for the replacer function
		 *    - The byte offsets for start-of-match and end-of-match to implement
		 *      the replacement values $&, $`, and $', and to copy non-matching
		 *      input string portions (including header and trailer) verbatim.
		 *
		 *  NOTE: the E5.1 specification is a bit vague how the RegExp should
		 *  behave in the replacement process; e.g. is matching done first for
		 *  all matches (in the global RegExp case) before any replacer calls
		 *  are made?  See: test-bi-string-proto-replace.js for discussion.
		 */

		DUK_ASSERT_TOP(ctx, 4);

#ifdef DUK_USE_REGEXP_SUPPORT
		if (is_regexp) {
			duk_dup(ctx, 0);
			duk_dup(ctx, 2);
			duk_regexp_match(thr);  /* [ ... regexp input ] -> [ res_obj ] */
			if (!duk_is_object(ctx, -1)) {
				duk_pop(ctx);
				break;
			}

			duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INDEX);
			DUK_ASSERT(duk_is_number(ctx, -1));
			match_start_coff = duk_get_int(ctx, -1);
			duk_pop(ctx);

			duk_get_prop_index(ctx, -1, 0);
			DUK_ASSERT(duk_is_string(ctx, -1));
			h_match = duk_get_hstring(ctx, -1);
			DUK_ASSERT(h_match != NULL);
			duk_pop(ctx);  /* h_match is borrowed, remains reachable through match_obj */

			if (DUK_HSTRING_GET_BYTELEN(h_match) == 0) {
				/* This should be equivalent to match() algorithm step 8.f.iii.2:
				 * detect an empty match and allow it, but don't allow it twice.
				 */
				duk_uint32_t last_index;

				duk_get_prop_stridx(ctx, 0, DUK_STRIDX_LAST_INDEX);
				last_index = (duk_uint32_t) duk_get_uint(ctx, -1);
				DUK_DDD(DUK_DDDPRINT("empty match, bump lastIndex: %ld -> %ld",
				                     (long) last_index, (long) (last_index + 1)));
				duk_pop(ctx);
				duk_push_int(ctx, last_index + 1);
				duk_put_prop_stridx(ctx, 0, DUK_STRIDX_LAST_INDEX);
			}

			DUK_ASSERT(duk_get_length(ctx, -1) <= DUK_INT_MAX);  /* string limits */
			match_caps = (duk_int_t) duk_get_length(ctx, -1);
		} else {
#else  /* DUK_USE_REGEXP_SUPPORT */
		{  /* unconditionally */
#endif  /* DUK_USE_REGEXP_SUPPORT */
			const duk_uint8_t *p_start, *p_end, *p;   /* input string scan */
			const duk_uint8_t *q_start;               /* match string */
			duk_size_t q_blen;

#ifdef DUK_USE_REGEXP_SUPPORT
			DUK_ASSERT(!is_global);  /* single match always */
#endif

			p_start = DUK_HSTRING_GET_DATA(h_input);
			p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_input);
			p = p_start;

			h_search = duk_get_hstring(ctx, 0);
			DUK_ASSERT(h_search != NULL);
			q_start = DUK_HSTRING_GET_DATA(h_search);
			q_blen = (duk_size_t) DUK_HSTRING_GET_BYTELEN(h_search);

			p_end -= q_blen;  /* ensure full memcmp() fits in while */

			match_start_coff = 0;

			while (p <= p_end) {
				DUK_ASSERT(p + q_blen <= DUK_HSTRING_GET_DATA(h_input) + DUK_HSTRING_GET_BYTELEN(h_input));
				if (DUK_MEMCMP((const void *) p, (const void *) q_start, (size_t) q_blen) == 0) {
					duk_dup(ctx, 0);
					h_match = duk_get_hstring(ctx, -1);
					DUK_ASSERT(h_match != NULL);
#ifdef DUK_USE_REGEXP_SUPPORT
					match_caps = 0;
#endif
					goto found;
				}

				/* track utf-8 non-continuation bytes */
				if ((p[0] & 0xc0) != 0x80) {
					match_start_coff++;
				}
				p++;
			}

			/* not found */
			break;
		}
	 found:

		/* stack[0] = search value
		 * stack[1] = replace value
		 * stack[2] = input string
		 * stack[3] = result buffer
		 * stack[4] = regexp match OR match string
		 */

		match_start_boff = duk_heap_strcache_offset_char2byte(thr, h_input, match_start_coff);

		tmp_sz = (duk_size_t) (match_start_boff - prev_match_end_boff);
		DUK_BW_WRITE_ENSURE_BYTES(thr, bw, DUK_HSTRING_GET_DATA(h_input) + prev_match_end_boff, tmp_sz);

		prev_match_end_boff = match_start_boff + DUK_HSTRING_GET_BYTELEN(h_match);

		if (is_repl_func) {
			duk_idx_t idx_args;
			duk_hstring *h_repl;

			/* regexp res_obj is at index 4 */

			duk_dup(ctx, 1);
			idx_args = duk_get_top(ctx);

#ifdef DUK_USE_REGEXP_SUPPORT
			if (is_regexp) {
				duk_int_t idx;
				duk_require_stack(ctx, match_caps + 2);
				for (idx = 0; idx < match_caps; idx++) {
					/* match followed by capture(s) */
					duk_get_prop_index(ctx, 4, idx);
				}
			} else {
#else  /* DUK_USE_REGEXP_SUPPORT */
			{  /* unconditionally */
#endif  /* DUK_USE_REGEXP_SUPPORT */
				/* match == search string, by definition */
				duk_dup(ctx, 0);
			}
			duk_push_int(ctx, match_start_coff);
			duk_dup(ctx, 2);

			/* [ ... replacer match [captures] match_char_offset input ] */

			duk_call(ctx, duk_get_top(ctx) - idx_args);
			h_repl = duk_to_hstring(ctx, -1);  /* -> [ ... repl_value ] */
			DUK_ASSERT(h_repl != NULL);

			DUK_BW_WRITE_ENSURE_HSTRING(thr, bw, h_repl);

			duk_pop(ctx);  /* repl_value */
		} else {
			r = r_start;

			while (r < r_end) {
				duk_int_t ch1;
				duk_int_t ch2;
#ifdef DUK_USE_REGEXP_SUPPORT
				duk_int_t ch3;
#endif
				duk_size_t left;

				ch1 = *r++;
				if (ch1 != DUK_ASC_DOLLAR) {
					goto repl_write;
				}
				left = r_end - r;

				if (left <= 0) {
					goto repl_write;
				}

				ch2 = r[0];
				switch ((int) ch2) {
				case DUK_ASC_DOLLAR: {
					ch1 = (1 << 8) + DUK_ASC_DOLLAR;
					goto repl_write;
				}
				case DUK_ASC_AMP: {
					DUK_BW_WRITE_ENSURE_HSTRING(thr, bw, h_match);
					r++;
					continue;
				}
				case DUK_ASC_GRAVE: {
					tmp_sz = (duk_size_t) match_start_boff;
					DUK_BW_WRITE_ENSURE_BYTES(thr, bw, DUK_HSTRING_GET_DATA(h_input), tmp_sz);
					r++;
					continue;
				}
				case DUK_ASC_SINGLEQUOTE: {
					duk_uint32_t match_end_boff;

					/* Use match charlen instead of bytelen, just in case the input and
					 * match codepoint encodings would have different lengths.
					 */
					match_end_boff = duk_heap_strcache_offset_char2byte(thr,
					                                                    h_input,
					                                                    match_start_coff + DUK_HSTRING_GET_CHARLEN(h_match));

					tmp_sz = (duk_size_t) (DUK_HSTRING_GET_BYTELEN(h_input) - match_end_boff);
					DUK_BW_WRITE_ENSURE_BYTES(thr, bw, DUK_HSTRING_GET_DATA(h_input) + match_end_boff, tmp_sz);
					r++;
					continue;
				}
				default: {
#ifdef DUK_USE_REGEXP_SUPPORT
					duk_int_t capnum, captmp, capadv;
					/* XXX: optional check, match_caps is zero if no regexp,
					 * so dollar will be interpreted literally anyway.
					 */

					if (!is_regexp) {
						goto repl_write;
					}

					if (!(ch2 >= DUK_ASC_0 && ch2 <= DUK_ASC_9)) {
						goto repl_write;
					}
					capnum = ch2 - DUK_ASC_0;
					capadv = 1;

					if (left >= 2) {
						ch3 = r[1];
						if (ch3 >= DUK_ASC_0 && ch3 <= DUK_ASC_9) {
							captmp = capnum * 10 + (ch3 - DUK_ASC_0);
							if (captmp < match_caps) {
								capnum = captmp;
								capadv = 2;
							}
						}
					}

					if (capnum > 0 && capnum < match_caps) {
						DUK_ASSERT(is_regexp != 0);  /* match_caps == 0 without regexps */

						/* regexp res_obj is at offset 4 */
						duk_get_prop_index(ctx, 4, (duk_uarridx_t) capnum);
						if (duk_is_string(ctx, -1)) {
							duk_hstring *h_tmp_str;

							h_tmp_str = duk_get_hstring(ctx, -1);
							DUK_ASSERT(h_tmp_str != NULL);

							DUK_BW_WRITE_ENSURE_HSTRING(thr, bw, h_tmp_str);
						} else {
							/* undefined -> skip (replaced with empty) */
						}
						duk_pop(ctx);
						r += capadv;
						continue;
					} else {
						goto repl_write;
					}
#else  /* DUK_USE_REGEXP_SUPPORT */
					goto repl_write;  /* unconditionally */
#endif  /* DUK_USE_REGEXP_SUPPORT */
				}  /* default case */
				}  /* switch (ch2) */

			 repl_write:
				/* ch1 = (r_increment << 8) + byte */

				DUK_BW_WRITE_ENSURE_U8(thr, bw, (duk_uint8_t) (ch1 & 0xff));
				r += ch1 >> 8;
			}  /* while repl */
		}  /* if (is_repl_func) */

		duk_pop(ctx);  /* pop regexp res_obj or match string */

#ifdef DUK_USE_REGEXP_SUPPORT
		if (!is_global) {
#else
		{  /* unconditionally; is_global==0 */
#endif
			break;
		}
	}

	/* trailer */
	tmp_sz = (duk_size_t) (DUK_HSTRING_GET_BYTELEN(h_input) - prev_match_end_boff);
	DUK_BW_WRITE_ENSURE_BYTES(thr, bw, DUK_HSTRING_GET_DATA(h_input) + prev_match_end_boff, tmp_sz);

	DUK_ASSERT_TOP(ctx, 4);
	DUK_BW_COMPACT(thr, bw);
	duk_to_string(ctx, -1);
	return 1;
}

/*
 *  split()
 */

/* XXX: very messy now, but works; clean up, remove unused variables (nomimally
 * used so compiler doesn't complain).
 */

DUK_INTERNAL duk_ret_t duk_bi_string_prototype_split(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_input;
	duk_hstring *h_sep;
	duk_uint32_t limit;
	duk_uint32_t arr_idx;
#ifdef DUK_USE_REGEXP_SUPPORT
	duk_bool_t is_regexp;
#endif
	duk_bool_t matched;  /* set to 1 if any match exists (needed for empty input special case) */
	duk_uint32_t prev_match_end_coff, prev_match_end_boff;
	duk_uint32_t match_start_boff, match_start_coff;
	duk_uint32_t match_end_boff, match_end_coff;

	DUK_UNREF(thr);

	h_input = duk_push_this_coercible_to_string(ctx);
	DUK_ASSERT(h_input != NULL);

	duk_push_array(ctx);

	if (duk_is_undefined(ctx, 1)) {
		limit = 0xffffffffUL;
	} else {
		limit = duk_to_uint32(ctx, 1);
	}

	if (limit == 0) {
		return 1;
	}

	/* If the separator is a RegExp, make a "clone" of it.  The specification
	 * algorithm calls [[Match]] directly for specific indices; we emulate this
	 * by tweaking lastIndex and using a "force global" variant of duk_regexp_match()
	 * which will use global-style matching even when the RegExp itself is non-global.
	 */

	if (duk_is_undefined(ctx, 0)) {
		/* The spec algorithm first does "R = ToString(separator)" before checking
		 * whether separator is undefined.  Since this is side effect free, we can
		 * skip the ToString() here.
		 */
		duk_dup(ctx, 2);
		duk_put_prop_index(ctx, 3, 0);
		return 1;
	} else if (duk_get_hobject_with_class(ctx, 0, DUK_HOBJECT_CLASS_REGEXP) != NULL) {
#ifdef DUK_USE_REGEXP_SUPPORT
		duk_push_hobject_bidx(ctx, DUK_BIDX_REGEXP_CONSTRUCTOR);
		duk_dup(ctx, 0);
		duk_new(ctx, 1);  /* [ ... RegExp val ] -> [ ... res ] */
		duk_replace(ctx, 0);
		/* lastIndex is initialized to zero by new RegExp() */
		is_regexp = 1;
#else
		return DUK_RET_UNSUPPORTED_ERROR;
#endif
	} else {
		duk_to_string(ctx, 0);
#ifdef DUK_USE_REGEXP_SUPPORT
		is_regexp = 0;
#endif
	}

	/* stack[0] = separator (string or regexp)
	 * stack[1] = limit
	 * stack[2] = input string
	 * stack[3] = result array
	 */

	prev_match_end_boff = 0;
	prev_match_end_coff = 0;
	arr_idx = 0;
	matched = 0;

	for (;;) {
		/*
		 *  The specification uses RegExp [[Match]] to attempt match at specific
		 *  offsets.  We don't have such a primitive, so we use an actual RegExp
		 *  and tweak lastIndex.  Since the RegExp may be non-global, we use a
		 *  special variant which forces global-like behavior for matching.
		 */

		DUK_ASSERT_TOP(ctx, 4);

#ifdef DUK_USE_REGEXP_SUPPORT
		if (is_regexp) {
			duk_dup(ctx, 0);
			duk_dup(ctx, 2);
			duk_regexp_match_force_global(thr);  /* [ ... regexp input ] -> [ res_obj ] */
			if (!duk_is_object(ctx, -1)) {
				duk_pop(ctx);
				break;
			}
			matched = 1;

			duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INDEX);
			DUK_ASSERT(duk_is_number(ctx, -1));
			match_start_coff = duk_get_int(ctx, -1);
			match_start_boff = duk_heap_strcache_offset_char2byte(thr, h_input, match_start_coff);
			duk_pop(ctx);

			if (match_start_coff == DUK_HSTRING_GET_CHARLEN(h_input)) {
				/* don't allow an empty match at the end of the string */
				duk_pop(ctx);
				break;
			}

			duk_get_prop_stridx(ctx, 0, DUK_STRIDX_LAST_INDEX);
			DUK_ASSERT(duk_is_number(ctx, -1));
			match_end_coff = duk_get_int(ctx, -1);
			match_end_boff = duk_heap_strcache_offset_char2byte(thr, h_input, match_end_coff);
			duk_pop(ctx);

			/* empty match -> bump and continue */
			if (prev_match_end_boff == match_end_boff) {
				duk_push_int(ctx, match_end_coff + 1);
				duk_put_prop_stridx(ctx, 0, DUK_STRIDX_LAST_INDEX);
				duk_pop(ctx);
				continue;
			}
		} else {
#else  /* DUK_USE_REGEXP_SUPPORT */
		{  /* unconditionally */
#endif  /* DUK_USE_REGEXP_SUPPORT */
			const duk_uint8_t *p_start, *p_end, *p;   /* input string scan */
			const duk_uint8_t *q_start;               /* match string */
			duk_size_t q_blen, q_clen;

			p_start = DUK_HSTRING_GET_DATA(h_input);
			p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_input);
			p = p_start + prev_match_end_boff;

			h_sep = duk_get_hstring(ctx, 0);
			DUK_ASSERT(h_sep != NULL);
			q_start = DUK_HSTRING_GET_DATA(h_sep);
			q_blen = (duk_size_t) DUK_HSTRING_GET_BYTELEN(h_sep);
			q_clen = (duk_size_t) DUK_HSTRING_GET_CHARLEN(h_sep);

			p_end -= q_blen;  /* ensure full memcmp() fits in while */

			match_start_coff = prev_match_end_coff;

			if (q_blen == 0) {
				/* Handle empty separator case: it will always match, and always
				 * triggers the check in step 13.c.iii initially.  Note that we
				 * must skip to either end of string or start of first codepoint,
				 * skipping over any continuation bytes!
				 *
				 * Don't allow an empty string to match at the end of the input.
				 */

				matched = 1;  /* empty separator can always match */

				match_start_coff++;
				p++;
				while (p < p_end) {
					if ((p[0] & 0xc0) != 0x80) {
						goto found;
					}
					p++;
				}
				goto not_found;
			}

			DUK_ASSERT(q_blen > 0 && q_clen > 0);
			while (p <= p_end) {
				DUK_ASSERT(p + q_blen <= DUK_HSTRING_GET_DATA(h_input) + DUK_HSTRING_GET_BYTELEN(h_input));
				DUK_ASSERT(q_blen > 0);  /* no issues with empty memcmp() */
				if (DUK_MEMCMP((const void *) p, (const void *) q_start, (size_t) q_blen) == 0) {
					/* never an empty match, so step 13.c.iii can't be triggered */
					goto found;
				}

				/* track utf-8 non-continuation bytes */
				if ((p[0] & 0xc0) != 0x80) {
					match_start_coff++;
				}
				p++;
			}

		 not_found:
			/* not found */
			break;

		 found:
			matched = 1;
			match_start_boff = (duk_uint32_t) (p - p_start);
			match_end_coff = (duk_uint32_t) (match_start_coff + q_clen);  /* constrained by string length */
			match_end_boff = (duk_uint32_t) (match_start_boff + q_blen);  /* ditto */

			/* empty match (may happen with empty separator) -> bump and continue */
			if (prev_match_end_boff == match_end_boff) {
				prev_match_end_boff++;
				prev_match_end_coff++;
				continue;
			}
		}  /* if (is_regexp) */

		/* stack[0] = separator (string or regexp)
		 * stack[1] = limit
		 * stack[2] = input string
		 * stack[3] = result array
		 * stack[4] = regexp res_obj (if is_regexp)
		 */

		DUK_DDD(DUK_DDDPRINT("split; match_start b=%ld,c=%ld, match_end b=%ld,c=%ld, prev_end b=%ld,c=%ld",
		                     (long) match_start_boff, (long) match_start_coff,
		                     (long) match_end_boff, (long) match_end_coff,
		                     (long) prev_match_end_boff, (long) prev_match_end_coff));

		duk_push_lstring(ctx,
		                 (const char *) (DUK_HSTRING_GET_DATA(h_input) + prev_match_end_boff),
		                 (duk_size_t) (match_start_boff - prev_match_end_boff));
		duk_put_prop_index(ctx, 3, arr_idx);
		arr_idx++;
		if (arr_idx >= limit) {
			goto hit_limit;
		}

#ifdef DUK_USE_REGEXP_SUPPORT
		if (is_regexp) {
			duk_size_t i, len;

			len = duk_get_length(ctx, 4);
			for (i = 1; i < len; i++) {
				DUK_ASSERT(i <= DUK_UARRIDX_MAX);  /* cannot have >4G captures */
				duk_get_prop_index(ctx, 4, (duk_uarridx_t) i);
				duk_put_prop_index(ctx, 3, arr_idx);
				arr_idx++;
				if (arr_idx >= limit) {
					goto hit_limit;
				}
			}

			duk_pop(ctx);
			/* lastIndex already set up for next match */
		} else {
#else  /* DUK_USE_REGEXP_SUPPORT */
		{  /* unconditionally */
#endif  /* DUK_USE_REGEXP_SUPPORT */
			/* no action */
		}

		prev_match_end_boff = match_end_boff;
		prev_match_end_coff = match_end_coff;
		continue;
	}  /* for */

	/* Combined step 11 (empty string special case) and 14-15. */

	DUK_DDD(DUK_DDDPRINT("split trailer; prev_end b=%ld,c=%ld",
	                     (long) prev_match_end_boff, (long) prev_match_end_coff));

	if (DUK_HSTRING_GET_CHARLEN(h_input) > 0 || !matched) {
		/* Add trailer if:
		 *   a) non-empty input
		 *   b) empty input and no (zero size) match found (step 11)
		 */

		duk_push_lstring(ctx,
		                 (const char *) DUK_HSTRING_GET_DATA(h_input) + prev_match_end_boff,
		                 (duk_size_t) (DUK_HSTRING_GET_BYTELEN(h_input) - prev_match_end_boff));
		duk_put_prop_index(ctx, 3, arr_idx);
		/* No arr_idx update or limit check */
	}

	return 1;

 hit_limit:
#ifdef DUK_USE_REGEXP_SUPPORT
	if (is_regexp) {
		duk_pop(ctx);
	}
#endif

	return 1;
}

/*
 *  Various
 */

#ifdef DUK_USE_REGEXP_SUPPORT
DUK_LOCAL void duk__to_regexp_helper(duk_context *ctx, duk_idx_t index, duk_bool_t force_new) {
	duk_hobject *h;

	/* Shared helper for match() steps 3-4, search() steps 3-4. */

	DUK_ASSERT(index >= 0);

	if (force_new) {
		goto do_new;
	}

	h = duk_get_hobject_with_class(ctx, index, DUK_HOBJECT_CLASS_REGEXP);
	if (!h) {
		goto do_new;
	}
	return;

 do_new:
	duk_push_hobject_bidx(ctx, DUK_BIDX_REGEXP_CONSTRUCTOR);
	duk_dup(ctx, index);
	duk_new(ctx, 1);  /* [ ... RegExp val ] -> [ ... res ] */
	duk_replace(ctx, index);
}
#endif  /* DUK_USE_REGEXP_SUPPORT */

#ifdef DUK_USE_REGEXP_SUPPORT
DUK_INTERNAL duk_ret_t duk_bi_string_prototype_search(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	/* Easiest way to implement the search required by the specification
	 * is to do a RegExp test() with lastIndex forced to zero.  To avoid
	 * side effects on the argument, "clone" the RegExp if a RegExp was
	 * given as input.
	 *
	 * The global flag of the RegExp should be ignored; setting lastIndex
	 * to zero (which happens when "cloning" the RegExp) should have an
	 * equivalent effect.
	 */

	DUK_ASSERT_TOP(ctx, 1);
	(void) duk_push_this_coercible_to_string(ctx);  /* at index 1 */
	duk__to_regexp_helper(ctx, 0 /*index*/, 1 /*force_new*/);

	/* stack[0] = regexp
	 * stack[1] = string
	 */

	/* Avoid using RegExp.prototype methods, as they're writable and
	 * configurable and may have been changed.
	 */

	duk_dup(ctx, 0);
	duk_dup(ctx, 1);  /* [ ... re_obj input ] */
	duk_regexp_match(thr);  /* -> [ ... res_obj ] */

	if (!duk_is_object(ctx, -1)) {
		duk_push_int(ctx, -1);
		return 1;
	}

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INDEX);
	DUK_ASSERT(duk_is_number(ctx, -1));
	return 1;
}
#else  /* DUK_USE_REGEXP_SUPPORT */
DUK_INTERNAL duk_ret_t duk_bi_string_prototype_search(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
#endif  /* DUK_USE_REGEXP_SUPPORT */

#ifdef DUK_USE_REGEXP_SUPPORT
DUK_INTERNAL duk_ret_t duk_bi_string_prototype_match(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_bool_t global;
	duk_int_t prev_last_index;
	duk_int_t this_index;
	duk_int_t arr_idx;

	DUK_ASSERT_TOP(ctx, 1);
	(void) duk_push_this_coercible_to_string(ctx);
	duk__to_regexp_helper(ctx, 0 /*index*/, 0 /*force_new*/);
	global = duk_get_prop_stridx_boolean(ctx, 0, DUK_STRIDX_GLOBAL, NULL);
	DUK_ASSERT_TOP(ctx, 2);

	/* stack[0] = regexp
	 * stack[1] = string
	 */

	if (!global) {
		duk_regexp_match(thr);  /* -> [ res_obj ] */
		return 1;  /* return 'res_obj' */
	}

	/* Global case is more complex. */

	/* [ regexp string ] */

	duk_push_int(ctx, 0);
	duk_put_prop_stridx(ctx, 0, DUK_STRIDX_LAST_INDEX);
	duk_push_array(ctx);

	/* [ regexp string res_arr ] */

	prev_last_index = 0;
	arr_idx = 0;

	for (;;) {
		DUK_ASSERT_TOP(ctx, 3);

		duk_dup(ctx, 0);
		duk_dup(ctx, 1);
		duk_regexp_match(thr);  /* -> [ ... regexp string ] -> [ ... res_obj ] */

		if (!duk_is_object(ctx, -1)) {
			duk_pop(ctx);
			break;
		}

		duk_get_prop_stridx(ctx, 0, DUK_STRIDX_LAST_INDEX);
		DUK_ASSERT(duk_is_number(ctx, -1));
		this_index = duk_get_int(ctx, -1);
		duk_pop(ctx);

		if (this_index == prev_last_index) {
			this_index++;
			duk_push_int(ctx, this_index);
			duk_put_prop_stridx(ctx, 0, DUK_STRIDX_LAST_INDEX);
		}
		prev_last_index = this_index;

		duk_get_prop_index(ctx, -1, 0);  /* match string */
		duk_put_prop_index(ctx, 2, arr_idx);
		arr_idx++;
		duk_pop(ctx);  /* res_obj */
	}

	if (arr_idx == 0) {
		duk_push_null(ctx);
	}

	return 1;  /* return 'res_arr' or 'null' */
}
#else  /* DUK_USE_REGEXP_SUPPORT */
DUK_INTERNAL duk_ret_t duk_bi_string_prototype_match(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
#endif  /* DUK_USE_REGEXP_SUPPORT */

DUK_INTERNAL duk_ret_t duk_bi_string_prototype_concat(duk_context *ctx) {
	/* duk_concat() coerces arguments with ToString() in correct order */
	(void) duk_push_this_coercible_to_string(ctx);
	duk_insert(ctx, 0);  /* this is relatively expensive */
	duk_concat(ctx, duk_get_top(ctx));
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_string_prototype_trim(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 0);
	(void) duk_push_this_coercible_to_string(ctx);
	duk_trim(ctx, 0);
	DUK_ASSERT_TOP(ctx, 1);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_string_prototype_locale_compare(duk_context *ctx) {
	duk_hstring *h1;
	duk_hstring *h2;
	duk_size_t h1_len, h2_len, prefix_len;
	duk_small_int_t ret = 0;
	duk_small_int_t rc;

	/* The current implementation of localeCompare() is simply a codepoint
	 * by codepoint comparison, implemented with a simple string compare
	 * because UTF-8 should preserve codepoint ordering (assuming valid
	 * shortest UTF-8 encoding).
	 *
	 * The specification requires that the return value must be related
	 * to the sort order: e.g. negative means that 'this' comes before
	 * 'that' in sort order.  We assume an ascending sort order.
	 */

	/* XXX: could share code with duk_js_ops.c, duk_js_compare_helper */

	h1 = duk_push_this_coercible_to_string(ctx);
	DUK_ASSERT(h1 != NULL);

	h2 = duk_to_hstring(ctx, 0);
	DUK_ASSERT(h2 != NULL);

	h1_len = (duk_size_t) DUK_HSTRING_GET_BYTELEN(h1);
	h2_len = (duk_size_t) DUK_HSTRING_GET_BYTELEN(h2);
	prefix_len = (h1_len <= h2_len ? h1_len : h2_len);

	/* Zero size compare not an issue with DUK_MEMCMP. */
	rc = (duk_small_int_t) DUK_MEMCMP((const void *) DUK_HSTRING_GET_DATA(h1),
	                                  (const void *) DUK_HSTRING_GET_DATA(h2),
	                                  (size_t) prefix_len);

	if (rc < 0) {
		ret = -1;
		goto done;
	} else if (rc > 0) {
		ret = 1;
		goto done;
	}

	/* prefix matches, lengths matter now */
	if (h1_len > h2_len) {
		ret = 1;
		goto done;
	} else if (h1_len == h2_len) {
		DUK_ASSERT(ret == 0);
		goto done;
	}
	ret = -1;
	goto done;

 done:
	duk_push_int(ctx, (duk_int_t) ret);
	return 1;
}
