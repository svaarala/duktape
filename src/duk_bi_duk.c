/*
 *  Duktape built-ins
 */

#include "duk_internal.h"

duk_ret duk_bi_duk_object_addr(duk_context *ctx) {
	duk_tval *tv;
	void *p;

	tv = duk_get_tval(ctx, 0);
	if (!tv || !DUK_TVAL_IS_HEAP_ALLOCATED(tv)) {
		return 0;  /* undefined */
	}
	p = (void *) DUK_TVAL_GET_HEAPHDR(tv);

	/* any heap allocated value (string, object, buffer) has a stable pointer */
	duk_push_sprintf(ctx, "%p", p);
	return 1;
}

duk_ret duk_bi_duk_object_refc(duk_context *ctx) {
#ifdef DUK_USE_REFERENCE_COUNTING
	duk_tval *tv = duk_get_tval(ctx, 0);
	duk_heaphdr *h;
	if (!tv) {
		return 0;
	}
	if (!DUK_TVAL_IS_HEAP_ALLOCATED(tv)) {
		return 0;
	}
	h = DUK_TVAL_GET_HEAPHDR(tv);
	duk_push_int(ctx, DUK_HEAPHDR_GET_REFCOUNT(h));
	return 1;
#else
	return 0;
#endif
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
	return DUK_RET_UNSUPPORTED_ERROR;
}
duk_ret duk_bi_duk_object_jx_enc(duk_context *ctx) {
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
	return DUK_RET_UNSUPPORTED_ERROR;
}
duk_ret duk_bi_duk_object_jc_enc(duk_context *ctx) {
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

