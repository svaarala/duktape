/*
 *  Conversions and coercions.
 *
 *  The conversion/coercions are in-place operations on the value stack.
 *  Some operations are implemented here directly, while others call a
 *  helper in duk_js_ops.c after validating arguments.
 */

#include "duk_internal.h"

/* E5 Section 8.12.8 */

static int defaultvalue_coerce_attempt(duk_context *ctx, int index, int func_stridx) {
	if (duk_get_prop_stridx(ctx, index, func_stridx)) {
		/* [ ... func ] */
		if (duk_is_callable(ctx, -1)) {
			duk_dup(ctx, index);         /* -> [ ... func this ] */
			duk_call_method(ctx, 0);     /* -> [ ... retval ] */
			if (duk_is_primitive(ctx, -1)) {
				duk_replace(ctx, index);
				return 1;
			}
			/* [ ... retval ]; popped below */
		}
	}
	duk_pop(ctx);  /* [ ... func/retval ] -> [ ... ] */
	return 0;
}

void duk_to_defaultvalue(duk_context *ctx, int index, int hint) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	int coercers[] = { DUK_HEAP_STRIDX_VALUE_OF, DUK_HEAP_STRIDX_TO_STRING };

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);

	index = duk_require_normalize_index(ctx, index);

	if (!duk_is_object(ctx, index)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "not object");
	}
	obj = duk_get_hobject(ctx, index);
	DUK_ASSERT(obj != NULL);

	if (hint == DUK_HINT_NONE) {
		if (DUK_HOBJECT_GET_CLASS_NUMBER(obj) == DUK_HOBJECT_CLASS_DATE) {
			hint = DUK_HINT_STRING;
		} else {
			hint = DUK_HINT_NUMBER;
		}
	}

	if (hint == DUK_HINT_STRING) {
		coercers[0] = DUK_HEAP_STRIDX_TO_STRING;
		coercers[1] = DUK_HEAP_STRIDX_VALUE_OF;
	}

	if (defaultvalue_coerce_attempt(ctx, index, coercers[0])) {
		return;
	}

	if (defaultvalue_coerce_attempt(ctx, index, coercers[1])) {
		return;
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "failed to coerce with [[DefaultValue]]");
}

void duk_to_undefined(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;

	DUK_ASSERT(ctx != NULL);
	thr = thr;  /* suppress warning */

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_UNDEFINED_ACTUAL(tv);
	DUK_TVAL_DECREF(thr, &tv_temp);
}

void duk_to_null(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;

	DUK_ASSERT(ctx != NULL);
	thr = thr;  /* suppress warning */

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NULL(tv);
	DUK_TVAL_DECREF(thr, &tv_temp);
}

/* E5 Section 9.1 */
void duk_to_primitive(duk_context *ctx, int index, int hint) {
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(hint == DUK_HINT_NONE || hint == DUK_HINT_NUMBER || hint == DUK_HINT_STRING);

	index = duk_require_normalize_index(ctx, index);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);

	if (DUK_TVAL_GET_TAG(tv) != DUK_TAG_OBJECT) {
		/* everything except object stay as is */
		return;
	}
	DUK_ASSERT(DUK_TVAL_IS_OBJECT(tv));

	duk_to_defaultvalue(ctx, index, hint);
}

/* E5 Section 9.2 */
int duk_to_boolean(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	int val;

	DUK_ASSERT(ctx != NULL);
	thr = thr;  /* suppress warning */

	index = duk_require_normalize_index(ctx, index);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);

	val = duk_js_toboolean(tv);

	/* Note: no need to re-lookup tv, conversion is side effect free */
	DUK_ASSERT(tv != NULL);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_BOOLEAN(tv, val);
	DUK_TVAL_DECREF(thr, &tv_temp);
	return val;
}

double duk_to_number(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	double d;

	DUK_ASSERT(ctx != NULL);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = duk_js_tonumber(thr, tv);

	/* Note: need to re-lookup because ToNumber() may have side effects */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_temp);

	return d;
}

/* FIXME: combine all the integer conversions: they share everything
 * but the helper function for coercion.
 */

int duk_to_int(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	double d;

	DUK_ASSERT(ctx != NULL);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = duk_js_tointeger(thr, tv);  /* E5 Section 9.4, ToInteger() */

	/* relookup in case duk_js_tointeger() ends up e.g. coercing an object */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_temp);

	return (int) d;
}

int duk_to_int32(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	double d;

	DUK_ASSERT(ctx != NULL);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = (double) duk_js_toint32(thr, tv);

	/* must relookup */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_temp);

	return (int) d;
}

unsigned int duk_to_uint32(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	double d;

	DUK_ASSERT(ctx != NULL);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = (double) duk_js_touint32(thr, tv);

	/* must relookup */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_temp);

	return (int) d;
}

unsigned int duk_to_uint16(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	double d;

	DUK_ASSERT(ctx != NULL);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = (double) duk_js_touint16(thr, tv);

	/* must relookup */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_temp);

	return (int) d;
}

const char *duk_to_lstring(duk_context *ctx, int index, size_t *out_len) {
	duk_to_string(ctx, index);
	return duk_require_lstring(ctx, index, out_len);
}

static void handle_to_string_number(duk_context *ctx, int index, duk_tval *tv) {
	duk_hthread *thr = (duk_hthread *) ctx;
	double d;
	int c;

	/*
	 *  FIXME: incorrect placeholder -- see E5 Section 9.8.1, very finicky
	 */

	d = DUK_TVAL_GET_NUMBER(tv);
	c = fpclassify(d);

	if (c == FP_INFINITE) {
		if (d < 0) {
			/* -Infinity */
			duk_push_hstring_stridx(ctx, DUK_HEAP_STRIDX_MINUS_INFINITY);
		} else {
			/* Infinity */
			duk_push_hstring_stridx(ctx, DUK_HEAP_STRIDX_INFINITY);
		}
	} else if (c == FP_NAN) {
		/* NaN */
		duk_push_hstring_stridx(ctx, DUK_HEAP_STRIDX_NAN);
	} else {
		if (DUK_TVAL_GET_NUMBER(tv) == (int) DUK_TVAL_GET_NUMBER(tv)) {
			duk_push_sprintf(ctx, "%d", (int) DUK_TVAL_GET_NUMBER(tv));
		} else if (DUK_TVAL_GET_NUMBER(tv) == (unsigned int) DUK_TVAL_GET_NUMBER(tv)) {
			duk_push_sprintf(ctx, "%u", (unsigned int) DUK_TVAL_GET_NUMBER(tv));
		} else {
			duk_push_sprintf(ctx, "%f", DUK_TVAL_GET_NUMBER(tv));
		}
	}
}

const char *duk_to_string(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

	index = duk_require_normalize_index(ctx, index);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED: {
		duk_push_hstring_stridx(ctx, DUK_HEAP_STRIDX_UNDEFINED);
		break;
	}
	case DUK_TAG_NULL: {
		duk_push_hstring_stridx(ctx, DUK_HEAP_STRIDX_NULL);
		break;
	}
	case DUK_TAG_BOOLEAN: {
		if (DUK_TVAL_GET_BOOLEAN(tv)) {
			duk_push_hstring_stridx(ctx, DUK_HEAP_STRIDX_TRUE);
		} else {
			duk_push_hstring_stridx(ctx, DUK_HEAP_STRIDX_FALSE);
		}
		break;
	}
	case DUK_TAG_STRING: {
		/* nop */
		goto skip_replace;
	}
	case DUK_TAG_OBJECT: {
		duk_to_primitive(ctx, index, DUK_HINT_STRING);
		return duk_to_string(ctx, index);  /* Note: recursive call */
	}
	case DUK_TAG_BUFFER: {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);

		/* Note: this currently allows creation of internal strings. */

		DUK_ASSERT(h != NULL);
		duk_push_lstring(ctx,
		                 (const char *) DUK_HBUFFER_GET_DATA_PTR(h),
		                 (unsigned int) DUK_HBUFFER_GET_SIZE(h));
		break;
	}
	case DUK_TAG_POINTER: {
		duk_push_sprintf(ctx, "%p", (void *) DUK_TVAL_GET_POINTER(tv));
		break;
	}
	default: {
		/* number */
		handle_to_string_number(ctx, index, tv);
		break;
	}
	}

	duk_replace(ctx, index);

 skip_replace:
	return duk_require_string(ctx, index);
}

/* internal */
duk_hstring *duk_to_hstring(duk_context *ctx, int index) {
	duk_hstring *ret;
	DUK_ASSERT(ctx != NULL);
	duk_to_string(ctx, index);
	ret = duk_get_hstring(ctx, index);
	DUK_ASSERT(ret != NULL);
	return ret;
}

void *duk_to_buffer(duk_context *ctx, int index) {
	DUK_ERROR((duk_hthread *) ctx, DUK_ERR_UNIMPLEMENTED_ERROR, "FIXME");
	return NULL;
}

void *duk_to_pointer(duk_context *ctx, int index) {
	DUK_ERROR((duk_hthread *) ctx, DUK_ERR_UNIMPLEMENTED_ERROR, "FIXME");
	return NULL;
}

void duk_to_object(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_hobject *res;

	DUK_ASSERT(ctx != NULL);

	index = duk_require_normalize_index(ctx, index);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL:
	case DUK_TAG_BUFFER:
	case DUK_TAG_POINTER: {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "attempt to coerce incompatible value to object");
		break;
	}
	case DUK_TAG_BOOLEAN: {
		int val = DUK_TVAL_GET_BOOLEAN(tv);

		(void) duk_push_new_object_helper(ctx,
		                                  DUK_HOBJECT_FLAG_EXTENSIBLE |
		                                  DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_BOOLEAN),
		                                  DUK_BIDX_BOOLEAN_PROTOTYPE);
		res = duk_require_hobject(ctx, -1);
		DUK_ASSERT(res != NULL);

		/* Note: Boolean prototype's internal value property is not writable,
		 * but duk_def_prop_stridx() disregards the write protection.
		 */
		duk_push_boolean(ctx, val);
		duk_def_prop_stridx(ctx, -2, DUK_HEAP_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);

		duk_replace(ctx, index);
		break;
	}
	case DUK_TAG_STRING: {
		(void) duk_push_new_object_helper(ctx,
		                                  DUK_HOBJECT_FLAG_EXTENSIBLE |
		                                  DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_STRING),
		                                  DUK_BIDX_STRING_PROTOTYPE);
		res = duk_require_hobject(ctx, -1);
		DUK_ASSERT(res != NULL);

		/* Note: String prototype's internal value property is not writable,
		 * but duk_def_prop_stridx() disregards the write protection.
		 */
		duk_dup(ctx, index);
		duk_def_prop_stridx(ctx, -2, DUK_HEAP_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);

		/* Enable special string behavior only after internal value has been set */
		DUK_HOBJECT_SET_SPECIAL_STRINGOBJ(res);

		duk_replace(ctx, index);
		break;
	}
	case DUK_TAG_OBJECT: {
		/* nop */
		break;
	}
	default: {
		/* number */
		double val = DUK_TVAL_GET_NUMBER(tv);

		(void) duk_push_new_object_helper(ctx,
		                                  DUK_HOBJECT_FLAG_EXTENSIBLE |
		                                  DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_NUMBER),
		                                  DUK_BIDX_NUMBER_PROTOTYPE);
		res = duk_require_hobject(ctx, -1);
		DUK_ASSERT(res != NULL);

		/* Note: Number prototype's internal value property is not writable,
		 * but duk_def_prop_stridx() disregards the write protection.
		 */
		duk_push_number(ctx, val);
		duk_def_prop_stridx(ctx, -2, DUK_HEAP_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);

		duk_replace(ctx, index);
		break;
	}
	}
}

/* FIXME: not really a conversion, move to another file */

const char *duk_to_base64(duk_context *ctx, int index) {
	const char *src;
	size_t srclen;
	size_t dstlen;
	const char *res;
	unsigned char *dst;

	index = duk_require_normalize_index(ctx, index);
	src = duk_require_lstring(ctx, index, &srclen);
	dstlen = (srclen + 2) / 3 * 4;
	dst = duk_push_new_fixed_buffer(ctx, dstlen);

	/* FIXME: zero size input? */

	duk_util_base64_encode((const unsigned char *) src, (unsigned char *) dst, srclen);

	/* FIXME: duk_to_string() to change buffer into string? */
	duk_push_lstring(ctx, (const char *) dst, dstlen);  /* -> [ ... buf res ] */
	duk_remove(ctx, -2);                 /* -> [ ... res ] */
	res = duk_require_string(ctx, -1);
	duk_replace(ctx, index);  /* -> [ ... ] */
	return res;
}



