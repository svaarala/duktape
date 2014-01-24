/*
 *  Duktape built-ins
 */

#include "duk_internal.h"

/* Raw helper to extract internal information / statistics about a value.
 * The return values are version specific and must not expose anything
 * that would lead to security issues (e.g. exposing compiled function
 * 'data' buffer might be an issue).  Currently only counts and sizes and
 * such are given so there should not be a security impact.
 */
duk_ret duk_bi_duk_object_info(duk_context *ctx) {
	duk_tval *tv;
	duk_heaphdr *h;
	duk_int_t i, n;

	tv = duk_get_tval(ctx, 0);
	DUK_ASSERT(tv != NULL);  /* because arg count is 1 */

	duk_push_array(ctx);  /* -> [ val arr ] */

	/* type tag (public) */
	duk_push_int(ctx, duk_get_type(ctx, 0));

	/* address */
	if (DUK_TVAL_IS_HEAP_ALLOCATED(tv)) {
		h = DUK_TVAL_GET_HEAPHDR(tv);
		duk_push_pointer(ctx, (void *) h);
	} else {
		h = NULL;
		duk_push_undefined(ctx);
	}

	/* refcount */
#ifdef DUK_USE_REFERENCE_COUNTING
	if (h) {
		duk_push_int(ctx, DUK_HEAPHDR_GET_REFCOUNT(h));
	} else {
		duk_push_undefined(ctx);
	}
#else
	duk_push_undefined(ctx);
#endif

	/* heaphdr size and additional allocation size, followed by
	 * type specific stuff (with varying value count)
	 */
	if (h) {
		switch (DUK_HEAPHDR_GET_TYPE(h)) {
		case DUK_HTYPE_STRING: {
			duk_hstring *h_str = (duk_hstring *) h;
			duk_push_int(ctx, (int) (sizeof(duk_hstring) + DUK_HSTRING_GET_BYTELEN(h_str) + 1));
			duk_push_undefined(ctx);
			break;
		}
		case DUK_HTYPE_OBJECT: {
			duk_hobject *h_obj = (duk_hobject *) h;
			duk_int_t hdr_size;
			if (DUK_HOBJECT_IS_COMPILEDFUNCTION(h_obj)) {
				hdr_size = (duk_int_t) sizeof(duk_hcompiledfunction);
			} else if (DUK_HOBJECT_IS_NATIVEFUNCTION(h_obj)) {
				hdr_size = (duk_int_t) sizeof(duk_hnativefunction);
			} else if (DUK_HOBJECT_IS_THREAD(h_obj)) {
				hdr_size = (duk_int_t) sizeof(duk_hthread);
			} else {
				hdr_size = (duk_int_t) sizeof(duk_hobject);
			}
			duk_push_int(ctx, (int) hdr_size);
			duk_push_int(ctx, (int) DUK_HOBJECT_E_ALLOC_SIZE(h_obj));
			duk_push_int(ctx, (int) h_obj->e_size);
			duk_push_int(ctx, (int) h_obj->e_used);
			duk_push_int(ctx, (int) h_obj->a_size);
			duk_push_int(ctx, (int) h_obj->h_size);
			if (DUK_HOBJECT_IS_COMPILEDFUNCTION(h_obj)) {
				duk_hbuffer *h_data = ((duk_hcompiledfunction *) h_obj)->data;
				if (h_data) {
					duk_push_int(ctx, DUK_HBUFFER_GET_SIZE(h_data));
				} else {
					duk_push_int(ctx, 0);
				}
			} else {
				duk_push_int(ctx, 0);
			}
			break;
		}
		case DUK_HTYPE_BUFFER: {
			duk_hbuffer *h_buf = (duk_hbuffer *) h;
			if (DUK_HBUFFER_HAS_DYNAMIC(h_buf)) {
				/* XXX: when usable_size == 0, dynamic buf ptr may now be NULL, in which case
				 * the second allocation does not exist.
				 */
				duk_hbuffer_dynamic *h_dyn = (duk_hbuffer_dynamic *) h;
				duk_push_int(ctx, (int) (sizeof(duk_hbuffer_dynamic)));
				duk_push_int(ctx, (int) (DUK_HBUFFER_DYNAMIC_GET_ALLOC_SIZE(h_dyn)));
			} else {
				duk_push_int(ctx, (int) (sizeof(duk_hbuffer_fixed) + DUK_HBUFFER_GET_SIZE(h_buf) + 1));
				duk_push_undefined(ctx);
			}
			break;
	
		}
		default: {
			duk_push_undefined(ctx);
			duk_push_undefined(ctx);
			break;
		}
		}
	} else {
		duk_push_undefined(ctx);
		duk_push_undefined(ctx);
	}

	/* set values into ret array */
	/* FIXME: primitive to make array from valstack slice */
	n = duk_get_top(ctx);
	for (i = 2; i < n; i++) {
		duk_dup(ctx, i);
		duk_put_prop_index(ctx, 1, i - 2);
	}
	duk_dup(ctx, 1);
	return 1;
}

duk_ret duk_bi_duk_object_gc(duk_context *ctx) {
#ifdef DUK_USE_MARK_AND_SWEEP
	duk_hthread *thr = (duk_hthread *) ctx;
	int flags;
	int rc;

	flags = duk_get_int(ctx, 0);
	rc = duk_heap_mark_and_sweep(thr->heap, flags);
	duk_push_int(ctx, rc);
	return 1;
#else
	return 0;
#endif
}

duk_ret duk_bi_duk_object_get_fin(duk_context *ctx) {
	(void) duk_require_hobject(ctx, 0);
	duk_get_prop_stridx(ctx, 0, DUK_STRIDX_INT_FINALIZER);
	return 1;
}

duk_ret duk_bi_duk_object_set_fin(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 2);
	(void) duk_put_prop_stridx(ctx, 0, DUK_STRIDX_INT_FINALIZER);  /* XXX: check value? */
	return 0;
}

duk_ret duk_bi_duk_object_enc(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_str;

	h_str = duk_to_hstring(ctx, 0);
	if (h_str == DUK_HTHREAD_STRING_HEX(thr)) {
		duk_hex_encode(ctx, 1);
		DUK_ASSERT_TOP(ctx, 2);
		return 1;
	} else if (h_str == DUK_HTHREAD_STRING_BASE64(thr)) {
		duk_base64_encode(ctx, 1);
		DUK_ASSERT_TOP(ctx, 2);
		return 1;
	} else {
		return DUK_RET_TYPE_ERROR;
	}
}

duk_ret duk_bi_duk_object_dec(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_str;

	h_str = duk_to_hstring(ctx, 0);
	if (h_str == DUK_HTHREAD_STRING_HEX(thr)) {
		duk_hex_decode(ctx, 1);
		DUK_ASSERT_TOP(ctx, 2);
		return 1;
	} else if (h_str == DUK_HTHREAD_STRING_BASE64(thr)) {
		duk_base64_decode(ctx, 1);
		DUK_ASSERT_TOP(ctx, 2);
		return 1;
	} else {
		return DUK_RET_TYPE_ERROR;
	}
}

#ifdef DUK_USE_JSONX
duk_ret duk_bi_duk_object_jx_dec(duk_context *ctx) {
	duk_bi_json_parse_helper(ctx,
	                         0 /*idx_value*/,
	                         1 /*idx_replacer*/,
	                         DUK_JSON_FLAG_EXT_CUSTOM /*flags*/);
	return 1;
}
duk_ret duk_bi_duk_object_jx_enc(duk_context *ctx) {
	duk_bi_json_stringify_helper(ctx,
	                             0 /*idx_value*/,
	                             1 /*idx_replacer*/,
	                             2 /*idx_space*/,
	                             DUK_JSON_FLAG_EXT_CUSTOM |
	                             DUK_JSON_FLAG_ASCII_ONLY |
	                             DUK_JSON_FLAG_AVOID_KEY_QUOTES /*flags*/);
	return 1;
}
#else  /* DUK_USE_JSONX */
duk_ret duk_bi_duk_object_jx_dec(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
duk_ret duk_bi_duk_object_jx_enc(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
#endif  /* DUK_USE_JSONX */

#ifdef DUK_USE_JSONC
duk_ret duk_bi_duk_object_jc_dec(duk_context *ctx) {
	duk_bi_json_parse_helper(ctx,
	                         0 /*idx_value*/,
	                         1 /*idx_replacer*/,
	                         DUK_JSON_FLAG_EXT_COMPATIBLE /*flags*/);
	return 1;
}
duk_ret duk_bi_duk_object_jc_enc(duk_context *ctx) {
	duk_bi_json_stringify_helper(ctx,
	                             0 /*idx_value*/,
	                             1 /*idx_replacer*/,
	                             2 /*idx_space*/,
	                             DUK_JSON_FLAG_EXT_COMPATIBLE |
	                             DUK_JSON_FLAG_ASCII_ONLY /*flags*/);
	return 1;
}
#else  /* DUK_USE_JSONC */
duk_ret duk_bi_duk_object_jc_dec(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
duk_ret duk_bi_duk_object_jc_enc(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
#endif  /* DUK_USE_JSONC */

/*
 *  Logging support
 */

#if 0
duk_ret duk_bi_duk_object_write_log(duk_context *ctx) {
	duk_double_t now;
	duk_int_t nargs;
	duk_int_t logger_level;
	duk_int_t message_level;

	/* this binding: logger
	 * magic: log level
	 * stack: plain log args
	 */

	nargs = duk_get_top(ctx);
	duk_push_this(ctx);  /* at idx nargs */

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_LEVEL);
	logger_level = duk_to_int(ctx, -1);

	message_level = duk_get_magic(ctx);

	if (message_level < logger_level) {
		return 0;
	}

	duk_push_int(ctx, message_level);

	/* [ args logger logger_level message_level ] */

	/* FIXME: here would be a point for pluggable backend */

	now = duk_bi_date_get_now(ctx);

	/* FIXME: stringify, sanitize ascii */

	/* FIXME: here would be another point for pluggable backend */

}
#endif

