/*
 *  Debugging related API calls
 */

#include "duk_internal.h"

DUK_EXTERNAL void duk_push_context_dump(duk_context *ctx) {
	duk_idx_t idx;
	duk_idx_t top;

	DUK_ASSERT_CTX_VALID(ctx);

	/* We don't duk_require_stack() here now, but rely on the caller having
	 * enough space.
	 */

	top = duk_get_top(ctx);
	duk_push_array(ctx);
	for (idx = 0; idx < top; idx++) {
		duk_dup(ctx, idx);
		duk_put_prop_index(ctx, -2, idx);
	}

	/* XXX: conversion errors should not propagate outwards.
	 * Perhaps values need to be coerced individually?
	 */
	duk_bi_json_stringify_helper(ctx,
	                             duk_get_top_index(ctx),  /*idx_value*/
	                             DUK_INVALID_INDEX,  /*idx_replacer*/
	                             DUK_INVALID_INDEX,  /*idx_space*/
	                             DUK_JSON_FLAG_EXT_CUSTOM |
	                             DUK_JSON_FLAG_ASCII_ONLY |
	                             DUK_JSON_FLAG_AVOID_KEY_QUOTES /*flags*/);

	duk_push_sprintf(ctx, "ctx: top=%ld, stack=%s", (long) top, (const char *) duk_safe_to_string(ctx, -1));
	duk_replace(ctx, -3);  /* [ ... arr jsonx(arr) res ] -> [ ... res jsonx(arr) ] */
	duk_pop(ctx);
	DUK_ASSERT(duk_is_string(ctx, -1));
}

#if defined(DUK_USE_DEBUGGER_SUPPORT)

DUK_EXTERNAL void duk_debugger_attach(duk_context *ctx,
                                      duk_debug_read_function read_cb,
                                      duk_debug_write_function write_cb,
                                      duk_debug_peek_function peek_cb,
                                      duk_debug_read_flush_function read_flush_cb,
                                      duk_debug_write_flush_function write_flush_cb,
                                      duk_debug_request_function request_cb,
                                      duk_debug_detached_function detached_cb,
                                      void *udata) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_heap *heap;
	const char *str;
	duk_size_t len;

	/* XXX: should there be an error or an automatic detach if
	 * already attached?
	 */

	DUK_D(DUK_DPRINT("application called duk_debugger_attach()"));

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(read_cb != NULL);
	DUK_ASSERT(write_cb != NULL);
	/* Other callbacks are optional. */

	heap = thr->heap;
	heap->dbg_read_cb = read_cb;
	heap->dbg_write_cb = write_cb;
	heap->dbg_peek_cb = peek_cb;
	heap->dbg_read_flush_cb = read_flush_cb;
	heap->dbg_write_flush_cb = write_flush_cb;
	heap->dbg_request_cb = request_cb;
	heap->dbg_detached_cb = detached_cb;
	heap->dbg_udata = udata;
	heap->dbg_have_next_byte = 0;

	/* Start in paused state. */
	heap->dbg_processing = 0;
	heap->dbg_paused = 1;
	heap->dbg_state_dirty = 1;
	heap->dbg_force_restart = 0;
	heap->dbg_step_type = 0;
	heap->dbg_step_thread = NULL;
	heap->dbg_step_csindex = 0;
	heap->dbg_step_startline = 0;
	heap->dbg_exec_counter = 0;
	heap->dbg_last_counter = 0;
	heap->dbg_last_time = 0.0;

	/* Send version identification and flush right afterwards.  Note that
	 * we must write raw, unframed bytes here.
	 */
	duk_push_sprintf(ctx, "%ld %ld %s %s\n",
	                 (long) DUK_DEBUG_PROTOCOL_VERSION,
	                 (long) DUK_VERSION,
	                 (const char *) DUK_GIT_DESCRIBE,
	                 (const char *) DUK_USE_TARGET_INFO);
	str = duk_get_lstring(ctx, -1, &len);
	DUK_ASSERT(str != NULL);
	duk_debug_write_bytes(thr, (const duk_uint8_t *) str, len);
	duk_debug_write_flush(thr);
	duk_pop(ctx);
}

DUK_EXTERNAL void duk_debugger_detach(duk_context *ctx) {
	duk_hthread *thr;

	DUK_D(DUK_DPRINT("application called duk_debugger_detach()"));

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);

	/* Can be called multiple times with no harm. */
	duk_debug_do_detach(thr->heap);
}

DUK_EXTERNAL void duk_debugger_cooperate(duk_context *ctx) {
	duk_hthread *thr;
	duk_bool_t processed_messages;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);

	if (!DUK_HEAP_IS_DEBUGGER_ATTACHED(thr->heap)) {
		return;
	}
	if (thr->callstack_top > 0 || thr->heap->dbg_processing) {
		/* Calling duk_debugger_cooperate() while Duktape is being
		 * called into is not supported.  This is not a 100% check
		 * but prevents any damage in most cases.
		 */
		return;
	}

	processed_messages = duk_debug_process_messages(thr, 1 /*no_block*/);
	DUK_UNREF(processed_messages);
}

DUK_EXTERNAL duk_bool_t duk_debugger_notify(duk_context *ctx, duk_idx_t nvalues) {
	duk_hthread *thr;
	duk_idx_t top;
	duk_idx_t idx;
	duk_bool_t ret = 0;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);

	DUK_D(DUK_DPRINT("application called duk_debugger_notify() with nvalues=%ld", (long) nvalues));

	top = duk_get_top(ctx);
	if (top < nvalues) {
		DUK_ERROR_RANGE(thr, "not enough stack values for notify");
		return ret;  /* unreachable */
	}
	if (DUK_HEAP_IS_DEBUGGER_ATTACHED(thr->heap)) {
		duk_debug_write_notify(thr, DUK_DBG_CMD_APPNOTIFY);
		for (idx = top - nvalues; idx < top; idx++) {
			duk_tval *tv = DUK_GET_TVAL_POSIDX(ctx, idx);
			duk_debug_write_tval(thr, tv);
		}
		duk_debug_write_eom(thr);

		/* Return non-zero (true) if we have a good reason to believe
		 * the notify was delivered; if we're still attached at least
		 * a transport error was not indicated by the transport write
		 * callback.  This is not a 100% guarantee of course.
		 */
		if (DUK_HEAP_IS_DEBUGGER_ATTACHED(thr->heap)) {
			ret = 1;
		}
	}
	duk_pop_n(ctx, nvalues);
	return ret;
}

DUK_EXTERNAL void duk_debugger_pause(duk_context *ctx) {
	duk_hthread *thr;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);

	DUK_D(DUK_DPRINT("application called duk_debugger_pause()"));

	/* Treat like a debugger statement: ignore when not attached. */
	if (DUK_HEAP_IS_DEBUGGER_ATTACHED(thr->heap)) {
		DUK_HEAP_SET_PAUSED(thr->heap);

		/* Pause on the next opcode executed.  This is always safe to do even
		 * inside the debugger message loop: the interrupt counter will be reset
		 * to its proper value when the message loop exits.
		 */
		DUK_HTHREAD_INTCTR_SET(thr, 1);
	}
}

#else  /* DUK_USE_DEBUGGER_SUPPORT */

DUK_EXTERNAL void duk_debugger_attach(duk_context *ctx,
                                      duk_debug_read_function read_cb,
                                      duk_debug_write_function write_cb,
                                      duk_debug_peek_function peek_cb,
                                      duk_debug_read_flush_function read_flush_cb,
                                      duk_debug_write_flush_function write_flush_cb,
                                      duk_debug_request_function request_cb,
                                      duk_debug_detached_function detached_cb,
                                      void *udata) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(read_cb);
	DUK_UNREF(write_cb);
	DUK_UNREF(peek_cb);
	DUK_UNREF(read_flush_cb);
	DUK_UNREF(write_flush_cb);
	DUK_UNREF(request_cb);
	DUK_UNREF(detached_cb);
	DUK_UNREF(udata);
	DUK_ERROR_TYPE((duk_hthread *) ctx, "no debugger support");
}

DUK_EXTERNAL void duk_debugger_detach(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ERROR_TYPE((duk_hthread *) ctx, "no debugger support");
}

DUK_EXTERNAL void duk_debugger_cooperate(duk_context *ctx) {
	/* nop */
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(ctx);
}

DUK_EXTERNAL duk_bool_t duk_debugger_notify(duk_context *ctx, duk_idx_t nvalues) {
	duk_idx_t top;

	DUK_ASSERT_CTX_VALID(ctx);

	top = duk_get_top(ctx);
	if (top < nvalues) {
		DUK_ERROR_RANGE_INVALID_COUNT((duk_hthread *) ctx);
		return 0;  /* unreachable */
	}

	/* No debugger support, just pop values. */
	duk_pop_n(ctx, nvalues);
	return 0;
}

DUK_EXTERNAL void duk_debugger_pause(duk_context *ctx) {
	/* Treat like debugger statement: nop */
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(ctx);
}

#endif  /* DUK_USE_DEBUGGER_SUPPORT */
