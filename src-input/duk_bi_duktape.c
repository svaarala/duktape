/*
 *  Duktape built-ins
 *
 *  Size optimization note: it might seem that vararg multipurpose functions
 *  like fin(), enc(), and dec() are not very size optimal, but using a single
 *  user-visible Ecmascript function saves a lot of run-time footprint; each
 *  Function instance takes >100 bytes.  Using a shared native helper and a
 *  'magic' value won't save much if there are multiple Function instances
 *  anyway.
 */

#include "duk_internal.h"

#if defined(DUK_USE_DUKTAPE_BUILTIN)

DUK_INTERNAL duk_ret_t duk_bi_duktape_object_info(duk_context *ctx) {
	(void) duk_inspect_value(ctx, -1);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_duktape_object_act(duk_context *ctx) {
	duk_int_t level;

	level = duk_to_int(ctx, 0);
	(void) duk_inspect_callstack_entry(ctx, level);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_duktape_object_gc(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_small_uint_t flags;
	duk_bool_t rc;

	flags = (duk_small_uint_t) duk_get_uint(ctx, 0);
	rc = duk_heap_mark_and_sweep(thr->heap, flags);

	/* XXX: Not sure what the best return value would be in the API.
	 * Return a boolean for now.  Note that rc == 0 is success (true).
	 */
	duk_push_boolean(ctx, !rc);
	return 1;
}

#if defined(DUK_USE_FINALIZER_SUPPORT)
DUK_INTERNAL duk_ret_t duk_bi_duktape_object_fin(duk_context *ctx) {
	(void) duk_require_hobject(ctx, 0);
	if (duk_get_top(ctx) >= 2) {
		/* Set: currently a finalizer is disabled by setting it to
		 * undefined; this does not remove the property at the moment.
		 * The value could be type checked to be either a function
		 * or something else; if something else, the property could
		 * be deleted.  Must use duk_set_finalizer() to keep
		 * DUK_HOBJECT_FLAG_HAVE_FINALIZER in sync.
		 */
		duk_set_top(ctx, 2);
		duk_set_finalizer(ctx, 0);
		return 0;
	} else {
		/* Get. */
		DUK_ASSERT(duk_get_top(ctx) == 1);
		duk_get_finalizer(ctx, 0);
		return 1;
	}
}
#endif  /* DUK_USE_FINALIZER_SUPPORT */

DUK_INTERNAL duk_ret_t duk_bi_duktape_object_enc(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_str;

	DUK_UNREF(thr);

	/* Vararg function: must be careful to check/require arguments.
	 * The JSON helpers accept invalid indices and treat them like
	 * non-existent optional parameters.
	 */

	h_str = duk_require_hstring(ctx, 0);  /* Could reject symbols, but no point: won't match comparisons. */
	duk_require_valid_index(ctx, 1);

	if (h_str == DUK_HTHREAD_STRING_HEX(thr)) {
		duk_set_top(ctx, 2);
		duk_hex_encode(ctx, 1);
		DUK_ASSERT_TOP(ctx, 2);
	} else if (h_str == DUK_HTHREAD_STRING_BASE64(thr)) {
		duk_set_top(ctx, 2);
		duk_base64_encode(ctx, 1);
		DUK_ASSERT_TOP(ctx, 2);
#if defined(DUK_USE_JSON_SUPPORT) && defined(DUK_USE_JX)
	} else if (h_str == DUK_HTHREAD_STRING_JX(thr)) {
		duk_bi_json_stringify_helper(ctx,
		                             1 /*idx_value*/,
		                             2 /*idx_replacer*/,
		                             3 /*idx_space*/,
		                             DUK_JSON_FLAG_EXT_CUSTOM |
		                             DUK_JSON_FLAG_ASCII_ONLY |
		                             DUK_JSON_FLAG_AVOID_KEY_QUOTES /*flags*/);
#endif
#if defined(DUK_USE_JSON_SUPPORT) && defined(DUK_USE_JC)
	} else if (h_str == DUK_HTHREAD_STRING_JC(thr)) {
		duk_bi_json_stringify_helper(ctx,
		                             1 /*idx_value*/,
		                             2 /*idx_replacer*/,
		                             3 /*idx_space*/,
		                             DUK_JSON_FLAG_EXT_COMPATIBLE |
		                             DUK_JSON_FLAG_ASCII_ONLY /*flags*/);
#endif
	} else {
		DUK_DCERROR_TYPE_INVALID_ARGS(thr);
	}
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_duktape_object_dec(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_str;

	DUK_UNREF(thr);

	/* Vararg function: must be careful to check/require arguments.
	 * The JSON helpers accept invalid indices and treat them like
	 * non-existent optional parameters.
	 */

	h_str = duk_require_hstring(ctx, 0);  /* Could reject symbols, but no point: won't match comparisons */
	duk_require_valid_index(ctx, 1);

	if (h_str == DUK_HTHREAD_STRING_HEX(thr)) {
		duk_set_top(ctx, 2);
		duk_hex_decode(ctx, 1);
		DUK_ASSERT_TOP(ctx, 2);
	} else if (h_str == DUK_HTHREAD_STRING_BASE64(thr)) {
		duk_set_top(ctx, 2);
		duk_base64_decode(ctx, 1);
		DUK_ASSERT_TOP(ctx, 2);
#if defined(DUK_USE_JSON_SUPPORT) && defined(DUK_USE_JX)
	} else if (h_str == DUK_HTHREAD_STRING_JX(thr)) {
		duk_bi_json_parse_helper(ctx,
		                         1 /*idx_value*/,
		                         2 /*idx_replacer*/,
		                         DUK_JSON_FLAG_EXT_CUSTOM /*flags*/);
#endif
#if defined(DUK_USE_JSON_SUPPORT) && defined(DUK_USE_JC)
	} else if (h_str == DUK_HTHREAD_STRING_JC(thr)) {
		duk_bi_json_parse_helper(ctx,
		                         1 /*idx_value*/,
		                         2 /*idx_replacer*/,
		                         DUK_JSON_FLAG_EXT_COMPATIBLE /*flags*/);
#endif
	} else {
		DUK_DCERROR_TYPE_INVALID_ARGS(thr);
	}
	return 1;
}

/*
 *  Compact an object
 */

DUK_INTERNAL duk_ret_t duk_bi_duktape_object_compact(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 1);
	duk_compact(ctx, 0);
	return 1;  /* return the argument object */
}

#endif  /* DUK_USE_DUKTAPE_BUILTIN */
