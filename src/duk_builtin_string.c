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
		duk_push_new_object_helper(ctx,
                           DUK_HOBJECT_FLAG_EXTENSIBLE |
		           DUK_HOBJECT_FLAG_SPECIAL_STRINGOBJ |
                           DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_STRING),
                           DUK_BIDX_STRING_PROTOTYPE);

		duk_dup(ctx, 0);
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);  /* FIXME: flags */
	}
	/* Note: unbalanced stack on purpose */

	return 1;
}

int duk_builtin_string_constructor_from_char_code(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hbuffer_growable *h;
	size_t i, n;
	unsigned int cp;

	/* FIXME: create a helper to create a UTF-8 string from multiple
	 * codepoints in one go.  Expose through C API?
	 */

	n = duk_get_top(ctx);
	duk_push_new_growable_buffer(ctx, 0);  /* FIXME: initial size estimate from 'n' */
	h = (duk_hbuffer_growable *) duk_get_hbuffer(ctx, -1);

	for (i = 0; i < n; i++) {
		cp = duk_to_uint16(ctx, i);
		duk_hbuffer_append_cesu8(thr, h, cp);
	}

	/* FIXME: rely here on arbitary buffer->string conversion */
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
	/* FIXME: can use same function */
	return duk_builtin_string_prototype_to_string(ctx);
}

/*
 *  Character and charcode access
 */

int duk_builtin_string_prototype_char_at(duk_context *ctx) {
	int pos;

	/* FIXME: faster implementation */
	/* FIXME: handling int values outside C int range */

	duk_push_this_coercible_to_string(ctx);
	pos = duk_to_int(ctx, 0);
	duk_substring(ctx, pos, pos + 1);
	return 1;
}

int duk_builtin_string_prototype_char_code_at(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int pos;
	duk_u32 boff;
	duk_hstring *h;
	duk_u8 *p, *p_start, *p_end;
	duk_u32 cp;

	/* FIXME: faster implementation */

	DUK_DDDPRINT("arg=%!T", duk_get_tval(ctx, 0));

	duk_push_this_coercible_to_string(ctx);
	h = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h != NULL);

	/* FIXME: need clamped check or clamp limits [-1, len+1] */
	pos = duk_to_int(ctx, 0);
	if (pos < 0 || pos >= DUK_HSTRING_GET_CHARLEN(h)) {
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

	duk_substring(ctx, (size_t) start_pos, (size_t) end_pos);
	return 1;
}

#ifdef DUK_USE_SECTION_B
int duk_builtin_string_prototype_substr(duk_context *ctx) {
	duk_hstring *h;
	int start_pos;
	int end_pos;
	int len;

	/* Unlike non-obsolete String calls, substr() will happily coerce
	 * undefined and null to strings.
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
	 * same behavior.  This is always obvious.
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

	duk_substring(ctx, (size_t) start_pos, (size_t) end_pos);
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

	duk_substring(ctx, (size_t) start_pos, (size_t) end_pos);
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

static int indexof_helper(duk_context *ctx, int is_lastindexof) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_this;
	duk_hstring *h_search;
	int clen_this;
	int blen_search;
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
	blen_search = DUK_HSTRING_GET_BYTELEN(h_search);
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
	if (blen_search == 0) {
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
	 * must be updated if 'p' is wound back.
	 */

	firstbyte = q_start[0];
	while (p <= p_end && p >= p_start) {
		t = *p;

		/* For Ecmascript strings, this check can only match for
		 * initial UTF-8 bytes (not continuation bytes).
		 */

		if ((t == firstbyte) && ((p_end - p) >= q_blen)) {
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
	return indexof_helper(ctx, 0 /*is_lastindexof*/);
}

int duk_builtin_string_prototype_last_index_of(duk_context *ctx) {
	/* -1 is used because the generated x86 load is shorter than for 1 */
	return indexof_helper(ctx, -1 /*is_lastindexof*/);
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
	duk_push_new_array(ctx);

	/* [ regexp string res_arr ] */

	prev_last_index = 0;
	arr_idx = 0;

	for (;;) {
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

		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_ZERO);  /* match string */
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
	duk_insert(ctx, 0);  /* XXX: this is relatively expensive */
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

int duk_builtin_string_prototype_replace(duk_context *ctx) {
	duk_push_this_coercible_to_string(ctx);
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_string_prototype_split(duk_context *ctx) {
	duk_push_this_coercible_to_string(ctx);
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
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

