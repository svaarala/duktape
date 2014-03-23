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

/* Raw helper to extract internal information / statistics about a value.
 * The return values are version specific and must not expose anything
 * that would lead to security issues (e.g. exposing compiled function
 * 'data' buffer might be an issue).  Currently only counts and sizes and
 * such are given so there should not be a security impact.
 */
duk_ret_t duk_bi_duk_object_info(duk_context *ctx) {
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
		goto done;
	}
	DUK_ASSERT(h != NULL);

	/* refcount */
#ifdef DUK_USE_REFERENCE_COUNTING
	duk_push_int(ctx, DUK_HEAPHDR_GET_REFCOUNT(h));
#else
	duk_push_undefined(ctx);
#endif

	/* heaphdr size and additional allocation size, followed by
	 * type specific stuff (with varying value count)
	 */
	switch (DUK_HEAPHDR_GET_TYPE(h)) {
	case DUK_HTYPE_STRING: {
		duk_hstring *h_str = (duk_hstring *) h;
		duk_push_int(ctx, (int) (sizeof(duk_hstring) + DUK_HSTRING_GET_BYTELEN(h_str) + 1));
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
		}
		break;

	}
	}

 done:
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

duk_ret_t duk_bi_duk_object_line(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_activation *act_caller;
	duk_hobject *h_func;
	duk_hbuffer_fixed *pc2line;
	duk_uint_fast32_t pc;
	duk_uint_fast32_t line;

	if (thr->callstack_top < 2) {
		return 0;
	}
	act_caller = thr->callstack + thr->callstack_top - 2;

	h_func = act_caller->func;
	DUK_ASSERT(h_func != NULL);
	if (!DUK_HOBJECT_HAS_COMPILEDFUNCTION(h_func)) {
		return 0;
	}

	/* FIXME: shared helper */
	duk_push_hobject(ctx, h_func);
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_PC2LINE);
	if (duk_is_buffer(ctx, -1)) {
		pc2line = (duk_hbuffer_fixed *) duk_get_hbuffer(ctx, -1);
		DUK_ASSERT(!DUK_HBUFFER_HAS_DYNAMIC((duk_hbuffer *) pc2line));
		pc = (duk_uint_fast32_t) act_caller->pc;
		line = duk_hobject_pc2line_query(pc2line, (duk_uint_fast32_t) pc);
	} else {
		line = 0;
	}

	duk_push_int(ctx, (int) line);  /* FIXME: typing */
	return 1;
}

duk_ret_t duk_bi_duk_object_gc(duk_context *ctx) {
#ifdef DUK_USE_MARK_AND_SWEEP
	duk_hthread *thr = (duk_hthread *) ctx;
	int flags;
	int rc;

	flags = duk_get_int(ctx, 0);
	rc = duk_heap_mark_and_sweep(thr->heap, flags);
	duk_push_int(ctx, rc);
	return 1;
#else
	DUK_UNREF(ctx);
	return 0;
#endif
}

duk_ret_t duk_bi_duk_object_fin(duk_context *ctx) {
	(void) duk_require_hobject(ctx, 0);
	if (duk_get_top(ctx) >= 2) {
		/* Set: currently a finalizer is disabled by setting it to
		 * undefined; this does not remove the property at the moment.
		 * The value could be type checked to be either a function
		 * or something else; if something else, the property could
		 * be deleted.
		 */
		duk_set_top(ctx, 2);
		(void) duk_put_prop_stridx(ctx, 0, DUK_STRIDX_INT_FINALIZER);
		return 0;
	} else {
		/* Get. */
		DUK_ASSERT(duk_get_top(ctx) == 1);
		duk_get_prop_stridx(ctx, 0, DUK_STRIDX_INT_FINALIZER);
		return 1;
	}
}

duk_ret_t duk_bi_duk_object_enc(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_str;

	/* Vararg function: must be careful to check/require arguments.
	 * The JSON helpers accept invalid indices and treat them like
	 * non-existent optional parameters.
	 */

	h_str = duk_require_hstring(ctx, 0);
	duk_require_valid_index(ctx, 1);

	if (h_str == DUK_HTHREAD_STRING_HEX(thr)) {
		duk_set_top(ctx, 2);
		duk_hex_encode(ctx, 1);
		DUK_ASSERT_TOP(ctx, 2);
	} else if (h_str == DUK_HTHREAD_STRING_BASE64(thr)) {
		duk_set_top(ctx, 2);
		duk_base64_encode(ctx, 1);
		DUK_ASSERT_TOP(ctx, 2);
#ifdef DUK_USE_JSONX
	} else if (h_str == DUK_HTHREAD_STRING_JSONX(thr)) {
		duk_bi_json_stringify_helper(ctx,
		                             1 /*idx_value*/,
		                             2 /*idx_replacer*/,
		                             3 /*idx_space*/,
		                             DUK_JSON_FLAG_EXT_CUSTOM |
		                             DUK_JSON_FLAG_ASCII_ONLY |
		                             DUK_JSON_FLAG_AVOID_KEY_QUOTES /*flags*/);
#endif
#ifdef DUK_USE_JSONC
	} else if (h_str == DUK_HTHREAD_STRING_JSONC(thr)) {
		duk_bi_json_stringify_helper(ctx,
		                             1 /*idx_value*/,
		                             2 /*idx_replacer*/,
		                             3 /*idx_space*/,
		                             DUK_JSON_FLAG_EXT_COMPATIBLE |
		                             DUK_JSON_FLAG_ASCII_ONLY /*flags*/);
#endif
	} else {
		return DUK_RET_TYPE_ERROR;
	}
	return 1;
}

duk_ret_t duk_bi_duk_object_dec(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_str;

	/* Vararg function: must be careful to check/require arguments.
	 * The JSON helpers accept invalid indices and treat them like
	 * non-existent optional parameters.
	 */

	h_str = duk_require_hstring(ctx, 0);
	duk_require_valid_index(ctx, 1);

	if (h_str == DUK_HTHREAD_STRING_HEX(thr)) {
		duk_set_top(ctx, 2);
		duk_hex_decode(ctx, 1);
		DUK_ASSERT_TOP(ctx, 2);
	} else if (h_str == DUK_HTHREAD_STRING_BASE64(thr)) {
		duk_set_top(ctx, 2);
		duk_base64_decode(ctx, 1);
		DUK_ASSERT_TOP(ctx, 2);
#ifdef DUK_USE_JSONX
	} else if (h_str == DUK_HTHREAD_STRING_JSONX(thr)) {
		duk_bi_json_parse_helper(ctx,
		                         1 /*idx_value*/,
		                         2 /*idx_replacer*/,
		                         DUK_JSON_FLAG_EXT_CUSTOM /*flags*/);
#endif
#ifdef DUK_USE_JSONC
	} else if (h_str == DUK_HTHREAD_STRING_JSONC(thr)) {
		duk_bi_json_parse_helper(ctx,
		                         1 /*idx_value*/,
		                         2 /*idx_replacer*/,
		                         DUK_JSON_FLAG_EXT_COMPATIBLE /*flags*/);
#endif
	} else {
		return DUK_RET_TYPE_ERROR;
	}
	return 1;
}

/*
 *  Compact an object
 */

duk_ret_t duk_bi_duk_object_compact(duk_context *ctx) {
	duk_compact(ctx, 0);
	return 1;  /* return the argument object */
}

