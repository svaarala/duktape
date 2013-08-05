/*
 *  String built-ins
 */

#include "duk_internal.h"

/*
 *  Constructor
 */

int duk_builtin_string_constructor(duk_context *ctx) {
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
		                       DUK_HOBJECT_FLAG_SPECIAL_STRINGOBJ |
		                       DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_STRING),
		                       DUK_BIDX_STRING_PROTOTYPE);

		/* String object internal value is immutable */
		duk_dup(ctx, 0);
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);
	}
	/* Note: unbalanced stack on purpose */

	return 1;
}

int duk_builtin_string_constructor_from_char_code(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hbuffer_dynamic *h;
	size_t i, n;
	unsigned int cp;

	/* FIXME: create a helper to create a UTF-8 string from multiple
	 * codepoints in one go.  Expose through C API?
	 */

	n = duk_get_top(ctx);
	duk_push_dynamic_buffer(ctx, 0);  /* FIXME: initial size estimate from 'n' */
	h = (duk_hbuffer_dynamic *) duk_get_hbuffer(ctx, -1);

	for (i = 0; i < n; i++) {
		cp = duk_to_uint16(ctx, i);
		duk_hbuffer_append_cesu8(thr, h, cp);
	}

	duk_to_string(ctx, -1);
	return 1;
}

/*
 *  toString(), valueOf()
 */

int duk_builtin_string_prototype_to_string(duk_context *ctx) {
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

int duk_builtin_string_prototype_value_of(duk_context *ctx) {
	/* FIXME: can use same native function */
	return duk_builtin_string_prototype_to_string(ctx);
}

/*
 *  Character and charcode access
 */

/* FIXME: charAt() and charCodeAt() could probably use a shared helper. */

int duk_builtin_string_prototype_char_at(duk_context *ctx) {
	int pos;

	/* FIXME: faster implementation */
	/* FIXME: handling int values outside C int range, currently
	 * duk_to_int() coerces to min/max int, so this works passably.
	 */

	duk_push_this_coercible_to_string(ctx);
	pos = duk_to_int(ctx, 0);
	duk_substring(ctx, -1, pos, pos + 1);
	return 1;
}

int duk_builtin_string_prototype_char_code_at(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int pos;
	duk_u32 boff;
	duk_hstring *h;
	duk_u8 *p, *p_start, *p_end;
	duk_u32 cp;
	int clamped;

	/* FIXME: faster implementation */

	DUK_DDDPRINT("arg=%!T", duk_get_tval(ctx, 0));

	duk_push_this_coercible_to_string(ctx);
	h = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h != NULL);

	pos = duk_to_int_clamped_raw(ctx,
	                             0 /*index*/,
	                             0 /*min(incl)*/,
	                             DUK_HSTRING_GET_CHARLEN(h) - 1 /*max(incl*/,
	                             &clamped /*clamped*/);
	if (clamped) {
		duk_push_number(ctx, NAN);  /* FIXME: best constant for NAN? */
		return 1;
	}

	boff = duk_heap_strcache_offset_char2byte(thr, h, (duk_u32) pos);
	DUK_DDDPRINT("charCodeAt: pos=%d -> boff=%d, str=%!O", pos, boff, h);
	DUK_ASSERT(boff >= 0 && boff < DUK_HSTRING_GET_BYTELEN(h));
	p_start = DUK_HSTRING_GET_DATA(h);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h);
	p = p_start + boff;
	DUK_DDDPRINT("p_start=%p, p_end=%p, p=%p", (void *) p_start, (void *) p_end, (void *) p);

	/* FIXME: this may throw an error, though not for valid E5 strings - is this OK here? */
	cp = duk_unicode_xutf8_get_u32_checked(thr, &p, p_start, p_end);

	/* FIXME: push_uint or push_u32 */
	duk_push_number(ctx, (double) cp);
	return 1;
}

/*
 *  substring(), substr(), slice()
 */

/* FIXME: any chance of merging these three similar algorithms? */

int duk_builtin_string_prototype_substring(duk_context *ctx) {
	duk_hstring *h;
	int start_pos;
	int end_pos;
	int len;

	duk_push_this_coercible_to_string(ctx);
	h = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h != NULL);
	len = DUK_HSTRING_GET_CHARLEN(h);

	/* [ start end str ] */

	/* FIXME: int clamping does not support full string range,
	 * needs type fixing.
	 */

	start_pos = duk_to_int_clamped(ctx, 0, 0, len);
	if (duk_is_undefined(ctx, 1)) {
		end_pos = len;
	} else {
		end_pos = duk_to_int_clamped(ctx, 1, 0, len);
	}
	DUK_ASSERT(start_pos >= 0 && start_pos <= len);
	DUK_ASSERT(end_pos >= 0 && end_pos <= len);

	if (start_pos > end_pos) {
		int tmp = start_pos;
		start_pos = end_pos;
		end_pos = tmp;
	}

	DUK_ASSERT(end_pos >= start_pos);

	duk_substring(ctx, -1, (size_t) start_pos, (size_t) end_pos);
	return 1;
}

#ifdef DUK_USE_SECTION_B
int duk_builtin_string_prototype_substr(duk_context *ctx) {
	duk_hstring *h;
	int start_pos;
	int end_pos;
	int len;

	/* Unlike non-obsolete String calls, substr() algorithm in E5.1
	 * specification will happily coerce undefined and null to strings
	 * ("undefined" and "null").
	 */
	duk_push_this(ctx);
	duk_to_string(ctx, -1);
	h = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h != NULL);
	len = DUK_HSTRING_GET_CHARLEN(h);

	/* [ start length str ] */

	/* FIXME: int clamping does not support full string range,
	 * needs type fixing.
	 */

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

	duk_substring(ctx, -1, (size_t) start_pos, (size_t) end_pos);
	return 1;
}
#endif  /* DUK_USE_SECTION_B */

int duk_builtin_string_prototype_slice(duk_context *ctx) {
	duk_hstring *h;
	int start_pos;
	int end_pos;
	int len;

	duk_push_this_coercible_to_string(ctx);
	h = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h != NULL);
	len = DUK_HSTRING_GET_CHARLEN(h);

	/* [ start end str ] */

	/* FIXME: int clamping does not support full string range,
	 * needs type fixing.
	 */

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

	duk_substring(ctx, -1, (size_t) start_pos, (size_t) end_pos);
	return 1;
}

/*
 *  Case conversion
 */

static int caseconv_helper(duk_context *ctx, int uppercase) {
	duk_hthread *thr = (duk_hthread *) ctx;

	duk_push_this_coercible_to_string(ctx);
	duk_unicode_case_convert_string(thr, uppercase);
	return 1;
}

int duk_builtin_string_prototype_to_lower_case(duk_context *ctx) {
	return caseconv_helper(ctx, 0 /*uppercase*/);
}

int duk_builtin_string_prototype_to_upper_case(duk_context *ctx) {
	return caseconv_helper(ctx, 1 /*uppercase*/);
}

int duk_builtin_string_prototype_to_locale_lower_case(duk_context *ctx) {
	/* Currently no locale specific case conversion */
	/* FIXME: use same native function */
	return duk_builtin_string_prototype_to_lower_case(ctx);
}

int duk_builtin_string_prototype_to_locale_upper_case(duk_context *ctx) {
	/* Currently no locale specific case conversion */
	/* FIXME: use same native function */
	return duk_builtin_string_prototype_to_upper_case(ctx);
}

/*
 *  indexOf() and lastIndexOf()
 */

static int string_indexof_helper(duk_context *ctx, int is_lastindexof) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_this;
	duk_hstring *h_search;
	int clen_this;
	int cpos;
	int bpos;
	duk_u8 *p_start, *p_end, *p;
	duk_u8 *q_start;
	size_t q_blen;
	duk_u8 firstbyte;
	duk_u8 t;

	duk_push_this_coercible_to_string(ctx);
	h_this = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h_this != NULL);
	clen_this = DUK_HSTRING_GET_CHARLEN(h_this);

	h_search = duk_to_hstring(ctx, 0);
	DUK_ASSERT(h_search != NULL);
	q_start = DUK_HSTRING_GET_DATA(h_search);
	q_blen = (size_t) DUK_HSTRING_GET_BYTELEN(h_search);

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

	/* Empty searchstring always matches; cpos must be clamped here. */
	if (q_blen == 0) {
		duk_push_int(ctx, cpos);
		return 1;
	}

	bpos = (int) duk_heap_strcache_offset_char2byte(thr, h_this, (duk_u32) cpos);

	p_start = DUK_HSTRING_GET_DATA(h_this);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_this);
	p = p_start + bpos;

	/* This loop is optimized for size.  For speed, there should be
	 * two separate loops, and we should ensure that memcmp() can be
	 * used without an extra "will searchstring fit" check.  Doing
	 * the preconditioning for 'p' and 'p_end' is easy but cpos
	 * must be updated if 'p' is wound back (backward scanning).
	 */

	firstbyte = q_start[0];
	while (p <= p_end && p >= p_start) {
		t = *p;

		/* For Ecmascript strings, this check can only match for
		 * initial UTF-8 bytes (not continuation bytes).
		 */

		if ((t == firstbyte) && ((p_end - p) >= q_blen)) {
			DUK_ASSERT(q_blen > 0);  /* no issues with memcmp() zero size, even if broken */
			if (memcmp(p, q_start, q_blen) == 0) {
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

int duk_builtin_string_prototype_index_of(duk_context *ctx) {
	return string_indexof_helper(ctx, 0 /*is_lastindexof*/);
}

int duk_builtin_string_prototype_last_index_of(duk_context *ctx) {
	/* -1 is used because the generated x86 load is shorter than for 1 */
	return string_indexof_helper(ctx, -1 /*is_lastindexof*/);
}

/*
 *  replace()
 */

/* FIXME: the current implementation works but is quite clunky; it compiles
 * to almost 1,5kB of x86 code so it needs to be simplified (better approach,
 * shared helpers, etc).
 */

/* FIXME: some ideas for refactoring:
 * - a primitive to convert a string into a regexp matcher (reduces matching
 *   code at the cost of making matching much slower)
 * - use replace() as a basic helper for match() and split(), which are both
 *   much simpler
 */

int duk_builtin_string_prototype_replace(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_input;
	duk_hstring *h_repl;
	duk_hstring *h_match;
	duk_hstring *h_search;
	duk_hobject *h_re;
	duk_hbuffer_dynamic *h_buf;
	int is_regexp;
	int is_global;
	int is_repl_func;
	duk_u32 match_start_coff, match_start_boff;
	int match_caps;
	duk_u32 prev_match_end_boff;
	duk_u8 *r_start, *r_end, *r;   /* repl string scan */

	DUK_ASSERT_TOP(ctx, 2);
	duk_push_this_coercible_to_string(ctx);
	h_input = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h_input != NULL);
	duk_push_dynamic_buffer(ctx, 0);
	h_buf = (duk_hbuffer_dynamic *) duk_get_hbuffer(ctx, -1);
	DUK_ASSERT(h_buf != NULL);
	DUK_ASSERT_TOP(ctx, 4);

	/* stack[0] = search value
	 * stack[1] = replace value
	 * stack[2] = input string
	 * stack[3] = result buffer
	 */

	h_re = duk_get_hobject_with_class(ctx, 0, DUK_HOBJECT_CLASS_REGEXP);
	if (h_re) {
		is_regexp = 1;
		duk_get_prop_stridx(ctx, 0, DUK_STRIDX_GLOBAL);
		DUK_ASSERT(duk_is_boolean(ctx, -1));
		is_global = duk_to_boolean(ctx, -1);
		duk_pop(ctx);

		if (is_global) {
			/* start match from beginning */
			duk_push_int(ctx, 0);
			duk_put_prop_stridx(ctx, 0, DUK_STRIDX_LAST_INDEX);
		}
	} else {
		duk_to_string(ctx, 0);
		is_regexp = 0;
		is_global = 0;
	}

	if (duk_is_function(ctx, 1)) {
		is_repl_func = 1;
		r_start = NULL;
		r_end = NULL;
	} else {
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
		 *  are made?  See: test-builtin-string-proto-replace.js for discussion.
		 */

		DUK_ASSERT_TOP(ctx, 4);

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
				duk_u32 last_index;

				duk_get_prop_stridx(ctx, 0, DUK_STRIDX_LAST_INDEX);
				last_index = duk_get_int(ctx, -1);  /* FIXME: duk_get_uint32() */
				DUK_DDDPRINT("empty match, bump lastIndex: %d -> %d", last_index, last_index + 1);
				duk_pop(ctx);
				duk_push_int(ctx, last_index + 1);
				duk_put_prop_stridx(ctx, 0, DUK_STRIDX_LAST_INDEX);
			}

			match_caps = duk_get_length(ctx, -1);
		} else {
			duk_u8 *p_start, *p_end, *p;   /* input string scan */
			duk_u8 *q_start;               /* match string */
			size_t q_blen;

			DUK_ASSERT(!is_global);  /* single match always */

			p_start = DUK_HSTRING_GET_DATA(h_input);
			p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_input);
			p = p_start;

			h_search = duk_get_hstring(ctx, 0);
			DUK_ASSERT(h_search != NULL);
			q_start = DUK_HSTRING_GET_DATA(h_search);
			q_blen = DUK_HSTRING_GET_BYTELEN(h_search);

			p_end -= q_blen;  /* ensure full memcmp() fits in while */

			match_start_coff = 0;

			while (p <= p_end) {
				/* FIXME: wrapped utility memcmp() which is guaranteed to work
				 * even if byte count is zero?
				 */
				DUK_ASSERT(p + q_blen <= DUK_HSTRING_GET_DATA(h_input) + DUK_HSTRING_GET_BYTELEN(h_input));
				if (memcmp((void *) p, (void *) q_start, (size_t) q_blen) == 0) {
					duk_dup(ctx, 0);
					h_match = duk_get_hstring(ctx, -1);
					DUK_ASSERT(h_match != NULL);
					match_caps = 0;
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

		duk_hbuffer_append_bytes(thr,
		                         h_buf,
		                         DUK_HSTRING_GET_DATA(h_input) + prev_match_end_boff,
		                         (size_t) (match_start_boff - prev_match_end_boff));

		prev_match_end_boff = match_start_boff + DUK_HSTRING_GET_BYTELEN(h_match);

		if (is_repl_func) {
			int idx_args;
			duk_hstring *h_repl;
			duk_u32 idx;

			/* regexp res_obj is at index 4 */

			duk_dup(ctx, 1);
			idx_args = duk_get_top(ctx);

			if (is_regexp) {
				duk_require_stack(ctx, match_caps + 2);
				for (idx = 0; idx < match_caps; idx++) {
					/* match followed by capture(s) */
					duk_get_prop_index(ctx, 4, idx);
				}
			} else {
				/* match == search string, by definition */
				duk_dup(ctx, 0);
			}
			duk_push_int(ctx, match_start_coff);
			duk_dup(ctx, 2);

			/* [ ... replacer match [captures] match_char_offset input ] */

			duk_call(ctx, duk_get_top(ctx) - idx_args);
			duk_to_string(ctx, -1);  /* -> [ ... repl_value ] */
			h_repl = duk_get_hstring(ctx, -1);
			DUK_ASSERT(h_repl != NULL);
			duk_hbuffer_append_hstring(thr, h_buf, h_repl);
			duk_pop(ctx);  /* repl_value */
		} else {
			r = r_start;

			while (r < r_end) {
				int ch1, ch2, ch3;
				int capnum, captmp, capadv;
				size_t left;

				ch1 = *r++;
				if (ch1 != (int) '$') {
					goto repl_write;
				}
				left = r_end - r;

				if (left <= 0) {
					goto repl_write;
				}

				ch2 = r[0];
				switch (ch2) {
				case (int) '$': {
					ch1 = (1 << 8) + (int) '$';
					goto repl_write;
				}
				case '&': {
					duk_hbuffer_append_hstring(thr, h_buf, h_match);
					r++;
					continue;
				}
				case '`': {
					duk_hbuffer_append_bytes(thr,
					                         h_buf,
					                         DUK_HSTRING_GET_DATA(h_input),
					                         match_start_boff);
					r++;
					continue;
				}
				case '\'': {
					duk_u32 match_end_boff;

					/* Use match charlen instead of bytelen, just in case the input and
					 * match codepoint encodings would have different lengths.
					 */
					match_end_boff = duk_heap_strcache_offset_char2byte(thr,
					                                                    h_input,
					                                                    match_start_coff + DUK_HSTRING_GET_CHARLEN(h_match));

					duk_hbuffer_append_bytes(thr,
					                         h_buf,
					                         DUK_HSTRING_GET_DATA(h_input) + match_end_boff,
					                         DUK_HSTRING_GET_BYTELEN(h_input) - match_end_boff);
					r++;
					continue;
				}
				default: {
					/* FIXME: optional check, match_caps is zero if no regexp,
					 * so dollar will be interpreted literally anyway.
					 */
					if (!is_regexp) {
						goto repl_write;
					}

					if (!(ch2 >= '0' && ch2 <= '9')) {
						goto repl_write;
					}
					capnum = ch2 - (int) '0';
					capadv = 1;

					if (left >= 2) {
						ch3 = r[1];
						if (ch3 >= '0' && ch3 <= '9') {
							captmp = capnum * 10 + (ch3 - (int) '0');
							if (captmp < match_caps) {
								capnum = captmp;
								capadv = 2;
							}
						}
					}

					if (capnum > 0 && capnum < match_caps) {
						DUK_ASSERT(is_regexp != 0);  /* match_caps == 0 without regexps */

						/* regexp res_obj is at offset 4 */
						duk_get_prop_index(ctx, 4, capnum);
						if (duk_is_string(ctx, -1)) {
							DUK_ASSERT(duk_get_hstring(ctx, -1) != NULL);
							duk_hbuffer_append_hstring(thr, h_buf, duk_get_hstring(ctx, -1));
						} else {
							/* undefined -> skip (replaced with empty) */
						}
						duk_pop(ctx);
						r += capadv;
						continue;
					} else {
						goto repl_write;
					}
				}  /* default case */
				}  /* switch(ch2) */

			 repl_write:
				/* ch1 = (r_increment << 8) + byte */
				duk_hbuffer_append_byte(thr, h_buf, (duk_u8) (ch1 & 0xff));
				r += ch1 >> 8;
			}  /* while repl */
		}

		duk_pop(ctx);  /* pop regexp res_obj or match string */

		if (!is_global) {
			break;
		}
	}

	/* trailer */
	duk_hbuffer_append_bytes(thr,
	                         h_buf,
	                         DUK_HSTRING_GET_DATA(h_input) + prev_match_end_boff,
	                         (size_t) (DUK_HSTRING_GET_BYTELEN(h_input) - prev_match_end_boff));

	DUK_ASSERT_TOP(ctx, 4);
	duk_to_string(ctx, -1);
	return 1;
}

/*
 *  split()
 */

/* FIXME: very messy now, but works */
/* FIXME: remove unused variables (they are nominally used so compiled doesn't complain) */
/* FIXME: general cleanup */

int duk_builtin_string_prototype_split(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_input;
	duk_hstring *h_sep;
	duk_u32 limit;
	duk_u32 arr_idx;
	int is_regexp;
	int matched;  /* set to 1 if any match exists (needed for empty input special case) */
	duk_u32 prev_match_end_coff, prev_match_end_boff;
	duk_u32 match_start_boff, match_start_coff;
	duk_u32 match_end_boff, match_end_coff;

	duk_push_this_coercible_to_string(ctx);
	h_input = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h_input != NULL);

	duk_push_array(ctx);

	if (duk_is_undefined(ctx, 1)) {
		limit = 0xffffffffU;
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
		duk_push_hobject(ctx, thr->builtins[DUK_BIDX_REGEXP_CONSTRUCTOR]);
		duk_dup(ctx, 0);
		duk_new(ctx, 1);  /* [ ... RegExp val ] -> [ ... res ] */
		duk_replace(ctx, 0);
		/* lastIndex is initialized to zero by new RegExp() */
		is_regexp = 1;
	} else {
		duk_to_string(ctx, 0);
		is_regexp = 0;
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

		if (is_regexp) {
			duk_dup(ctx, 0);
			duk_dup(ctx, 2);
			duk_regexp_match_force_global(ctx);  /* [ ... regexp input ] -> [ res_obj ] */
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
			duk_u8 *p_start, *p_end, *p;   /* input string scan */
			duk_u8 *q_start;               /* match string */
			size_t q_blen, q_clen;

			p_start = DUK_HSTRING_GET_DATA(h_input);
			p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_input);
			p = p_start + prev_match_end_boff;

			h_sep = duk_get_hstring(ctx, 0);
			DUK_ASSERT(h_sep != NULL);
			q_start = DUK_HSTRING_GET_DATA(h_sep);
			q_blen = DUK_HSTRING_GET_BYTELEN(h_sep);
			q_clen = DUK_HSTRING_GET_CHARLEN(h_sep);

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
				if (memcmp((void *) p, (void *) q_start, (size_t) q_blen) == 0) {
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
			match_start_boff = (duk_u32) (p - p_start);
			match_end_coff = match_start_coff + q_clen;
			match_end_boff = match_start_boff + q_blen;

			/* empty match (may happen with empty separator) -> bump and continue */
			if (prev_match_end_boff == match_end_boff) {
				prev_match_end_boff++;
				prev_match_end_coff++;
				continue;
			}
		}

		/* stack[0] = separator (string or regexp)
		 * stack[1] = limit
		 * stack[2] = input string
		 * stack[3] = result array
		 * stack[4] = regexp res_obj (if is_regexp)
		 */

		DUK_DDDPRINT("split; match_start b=%d,c=%d, match_end b=%d,c=%d, prev_end b=%d,c=%d",
		             (int) match_start_boff, (int) match_start_coff,
		             (int) match_end_boff, (int) match_end_coff,
		             (int) prev_match_end_boff, (int) prev_match_end_coff);

		duk_push_lstring(ctx,
		                 (const char *) (DUK_HSTRING_GET_DATA(h_input) + prev_match_end_boff),
		                 (size_t) (match_start_boff - prev_match_end_boff));
		duk_put_prop_index(ctx, 3, arr_idx);
		arr_idx++;
		if (arr_idx >= limit) {
			goto hit_limit;
		}

		if (is_regexp) {
			size_t i, len;

			len = duk_get_length(ctx, 4);
			for (i = 1; i < len; i++) {
				duk_get_prop_index(ctx, 4, i);
				duk_put_prop_index(ctx, 3, arr_idx);
				arr_idx++;
				if (arr_idx >= limit) {
					goto hit_limit;
				}
			}

			duk_pop(ctx);
			/* lastIndex already set up for next match */
		} else {
			/* no action */
		}

		prev_match_end_boff = match_end_boff;
		prev_match_end_coff = match_end_coff;
		continue;
	}

	/* Combined step 11 (empty string special case) and 14-15. */

	DUK_DDDPRINT("split trailer; match_start b=%d,c=%d, match_end b=%d,c=%d, prev_end b=%d,c=%d",
	             (int) match_start_boff, (int) match_start_coff,
	             (int) match_end_boff, (int) match_end_coff,
	             (int) prev_match_end_boff, (int) prev_match_end_coff);

	if (DUK_HSTRING_GET_CHARLEN(h_input) > 0 || !matched) {
		/* Add trailer if:
		 *   a) non-empty input
		 *   b) empty input and no (zero size) match found (step 11)
		 */

		duk_push_lstring(ctx,
		                 (const char *) DUK_HSTRING_GET_DATA(h_input) + prev_match_end_boff,
		                 (size_t) (DUK_HSTRING_GET_BYTELEN(h_input) - prev_match_end_boff));
		duk_put_prop_index(ctx, 3, arr_idx);
		/* No arr_idx update or limit check */
	}

	return 1;

 hit_limit:
	if (is_regexp) {
		duk_pop(ctx);
	}

	return 1;
}

/*
 *  Various
 */

static void to_regexp_helper(duk_context *ctx, int index, int force_new) {
	duk_hthread *thr = (duk_hthread *) ctx;
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
	duk_push_hobject(ctx, thr->builtins[DUK_BIDX_REGEXP_CONSTRUCTOR]);
	duk_dup(ctx, index);
	duk_new(ctx, 1);  /* [ ... RegExp val ] -> [ ... res ] */
	duk_replace(ctx, index);
}

int duk_builtin_string_prototype_search(duk_context *ctx) {
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
	duk_push_this_coercible_to_string(ctx);  /* at index 1 */
	to_regexp_helper(ctx, 0 /*index*/, 1 /*force_new*/);

	/* stack[0] = regexp
	 * stack[1] = string
	 */

	/* Avoid using RegExp.prototype methods, as they're writable and
	 * configurable.
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

int duk_builtin_string_prototype_match(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int global;
	int prev_last_index;
	int this_index;
	int arr_idx;

	DUK_ASSERT_TOP(ctx, 1);
	duk_push_this_coercible_to_string(ctx);
	to_regexp_helper(ctx, 0 /*index*/, 0 /*force_new*/);
	duk_get_prop_stridx(ctx, 0, DUK_STRIDX_GLOBAL);
	DUK_ASSERT(duk_is_boolean(ctx, -1));  /* 'global' is non-configurable and non-writable */
	global = duk_get_boolean(ctx, -1);
	duk_pop(ctx);
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

int duk_builtin_string_prototype_concat(duk_context *ctx) {
	/* duk_concat() coerces arguments with ToString() in correct order */
	duk_push_this_coercible_to_string(ctx);
	duk_insert(ctx, 0);  /* FIXME: this is relatively expensive */
	duk_concat(ctx, duk_get_top(ctx));
	return 1;
}

int duk_builtin_string_prototype_trim(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 0);
	duk_push_this_coercible_to_string(ctx);
	duk_trim(ctx, 0);
	DUK_ASSERT_TOP(ctx, 1);
	return 1;
}

int duk_builtin_string_prototype_locale_compare(duk_context *ctx) {
	duk_hstring *h1;
	duk_hstring *h2;
	size_t h1_len, h2_len, prefix_len;
	int rc;
	int ret = 0;

	/* The current implementation of localeCompare() is simply a codepoint
	 * by codepoint comparison, implemented with a simple string compare
	 * because UTF-8 should preserve codepoint ordering (assuming valid
	 * shortest UTF-8 encoding).
	 *
	 * The specification requires that the return value must be related
	 * to the sort order: e.g. negative means that 'this' comes before
	 * 'that' in sort order.  We assume an ascending sort order.
	 */

	/* FIXME: could share code with duk_js_ops.c, duk_js_compare_helper */

	duk_push_this_coercible_to_string(ctx);
	h1 = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h1 != NULL);

	h2 = duk_to_hstring(ctx, 0);
	DUK_ASSERT(h2 != NULL);

	h1_len = DUK_HSTRING_GET_BYTELEN(h1);
	h2_len = DUK_HSTRING_GET_BYTELEN(h2);
	prefix_len = (h1_len <= h2_len ? h1_len : h2_len);

	/* memcmp() should return zero (equal) for zero length, but avoid
	 * it because there are some platform specific bugs.  Don't use
	 * strncmp() because it stops comparing at a NUL.
	 */

	if (prefix_len == 0) {
		rc = 0;
		goto skip_memcmp;
	}
	rc = memcmp((const char *) DUK_HSTRING_GET_DATA(h1),
	            (const char *) DUK_HSTRING_GET_DATA(h2),
	            prefix_len);
 skip_memcmp:

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
	duk_push_int(ctx, ret);
	return 1;
}

