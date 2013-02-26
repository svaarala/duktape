/*
 *  String built-ins
 */

#include "duk_internal.h"

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

int duk_builtin_string_prototype_char_at(duk_context *ctx) {
	int pos;

	/* FIXME: faster implementation */
	/* FIXME: refactor shared parts, like "push_this + objectcoercible + to_string" */
	/* FIXME: CheckObjectCoercible check; call a helper */

	duk_push_this(ctx);
	DUK_DDDPRINT("CHARAT: this=%!T", duk_get_tval(ctx, -1));
	if (duk_is_null_or_undefined(ctx, -1)) {
		return DUK_RET_TYPE_ERROR;
	}

	duk_to_string(ctx, -1);
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
	/* FIXME: refactor shared parts, like "push_this + objectcoercible + to_string" */
	/* FIXME: CheckObjectCoercible check missing */
	/* FIXME: probably want in the API too */

	DUK_DDDPRINT("arg=%!T", duk_get_tval(ctx, 0));

	duk_push_this(ctx);
	duk_to_string(ctx, -1);
	h = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h != NULL);

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
	cp = duk_unicode_xutf8_get_u32(thr, &p, p_start, p_end);

	/* FIXME: push_uint or push_u32 */
	duk_push_number(ctx, (double) cp);
	return 1;
}

int duk_builtin_string_prototype_concat(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_string_prototype_index_of(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_string_prototype_last_index_of(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_string_prototype_locale_compare(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_string_prototype_match(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_string_prototype_replace(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_string_prototype_search(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_string_prototype_slice(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_string_prototype_split(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_string_prototype_substring(duk_context *ctx) {
	duk_hstring *h;
	int start_pos;
	int end_pos;
	int len;

	/* FIXME: CheckObjectCoercible check missing */
	/* FIXME: refactor shared parts, like "push_this + objectcoercible + to_string" */
	/* FIXME: probably want in the API too */

	duk_push_this(ctx);
	duk_to_string(ctx, -1);
	h = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h != NULL);
	len = DUK_HSTRING_GET_CHARLEN(h);

	/* [ start end str ] */

	/* FIXME: this coercion works incorrectly for number values outside
	 * integer range; e.g. Number.POSITIVE_INFINITY as an endpoint must
	 * clamp to 'len'.
	 */

	start_pos = duk_to_int(ctx, 0);
	if (duk_is_undefined(ctx, 1)) {
		end_pos = len;
	} else {
		end_pos = duk_to_int(ctx, 1);
	}

	if (start_pos < 0) {
		start_pos = 0;
	} else if (start_pos >= len) {
		start_pos = len;
	}

	if (end_pos < 0) {
		end_pos = 0;
	} else if (end_pos >= len) {
		end_pos = len;
	}

	if (start_pos > end_pos) {
		int tmp = start_pos;
		start_pos = end_pos;
		end_pos = tmp;
	}

	duk_substring(ctx, (size_t) start_pos, (size_t) end_pos);
	return 1;
}

int duk_builtin_string_prototype_to_lower_case(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	duk_push_this(ctx);

#if 0
	/* FIXME: a combined "check coercible + to object" helper? */
	if (!duk_is_object_coercible(ctx, -1)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "invalid arg");
	}
#endif
	duk_to_string(ctx, -1);

	duk_unicode_case_convert_string(thr, 0 /*uppercase*/);
	return 1;
}

int duk_builtin_string_prototype_to_locale_lower_case(duk_context *ctx) {
	/* FIXME */
	return duk_builtin_string_prototype_to_lower_case(ctx);
}

int duk_builtin_string_prototype_to_upper_case(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	duk_push_this(ctx);

#if 0
	/* FIXME: a combined "check coercible + to object" helper? */
	if (!duk_is_object_coercible(ctx, -1)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "invalid arg");
	}
#endif

	duk_to_string(ctx, -1);

	duk_unicode_case_convert_string(thr, 1 /*uppercase*/);
	return 1;
}

int duk_builtin_string_prototype_to_locale_upper_case(duk_context *ctx) {
	/* FIXME */
	return duk_builtin_string_prototype_to_upper_case(ctx);
}

int duk_builtin_string_prototype_trim(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

#if 1  /* FIXME: section B */
int duk_builtin_string_prototype_substr(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;
}
#endif

