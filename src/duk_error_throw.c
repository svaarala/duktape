/*
 *  Create and throw an Ecmascript error object based on a code and a message.
 *
 *  Used when we throw errors internally.  Ecmascript generated error objects
 *  are created by Ecmascript code, and the throwing is handled by the bytecode
 *  executor.
 */

#include "duk_internal.h"

/*
 *  Helper for calling errhandler.
 *
 *  'thr' must be the currently active thread; the errhandler is called
 *  in its context.  The valstack of 'thr' must have the error value on
 *  top, and will be replaced by another error value based on the return
 *  value of the errhandler.
 *
 *  The helper calls duk_handle_call() recursively in protected mode.
 *  Before that call happens, no longjmps should happen; as a consequence,
 *  we must assume that the valstack contains enough temporary space for
 *  arguments and such.
 *
 *  While the error handler runs, any errors thrown will not trigger a
 *  recursive error handler call (this is implemented using a heap level
 *  flag which will "follow" through any coroutines resumed inside the
 *  error handler).  If the error handler is not callable or throws an
 *  error, a DoubleError replaces the original error at the moment.
 *  This would be easy to change and even signal to the caller.
 *
 *  The current error handler is stored in 'Duktape.errhnd'.  There are
 *  several alternatives to this approach, such as: internal storage
 *  (thr->errhandler), the value stack (in a caller's frame), global object,
 *  global stash, thread object, thread stash.  The Duktape object is easy
 *  to access from both C and Ecmascript code without additional API calls,
 *  and is relatively good for sandboxing because one can just delete the
 *  globally reachable reference to the Duktape object (perhaps after
 *  migrating it to a stash).  The error handler is also automatically
 *  effective in a resumed thread, which is probably a good default behavior.
 *
 *  Note: since further longjmp()s may occur while calling the errhandler
 *  (for many reasons, e.g. a labeled 'break' inside the handler), the
 *  caller can make no assumptions on the thr->heap->lj state after the
 *  call.  This is currently not an issue, because the lj state is only
 *  written after the errhandler finishes.
 */

void duk_err_call_errhandler(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_tval *tv_hnd;
	int call_flags;
	int rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);

	if (DUK_HEAP_HAS_ERRHANDLER_RUNNING(thr->heap)) {
		DUK_DDPRINT("recursive call to errhandler, ignore");
		return;
	}

	/*
	 *  Check whether or not we have an error handler.
	 *
	 *  We must be careful of not triggering an error when looking up the
	 *  property.  For instance, if the property is a getter, we don't want
	 *  to call it, only plain values are allowed.  The value, if it exists,
	 *  is not checked.  If the value is not a function, a TypeError happens
	 *  when it is called and that error replaces the original one.
	 */

	DUK_ASSERT_VALSTACK_SPACE(thr, 4);  /* 4 entries needed below */

	/* [ ... errval ] */

	tv_hnd = duk_hobject_find_existing_entry_tval_ptr(thr->builtins[DUK_BIDX_DUKTAPE],
	                                                  DUK_HTHREAD_STRING_ERRHND(thr));
	if (tv_hnd == NULL) {
		DUK_DDPRINT("errhandler does not exist or is not a plain value: %!T", tv_hnd);
		return;
	}
	DUK_DDDPRINT("errhandler dump (callability not checked): %!T", tv_hnd);
	duk_push_tval(ctx, tv_hnd);

	/* [ ... errval errhandler ] */

	duk_insert(ctx, -2);  /* -> [ ... errhandler errval ] */
	duk_push_undefined(ctx);
	duk_insert(ctx, -2);  /* -> [ ... errhandler undefined(= this) errval ] */

	/* [ ... errhandler undefined errval ] */

	/*
	 *  DUK_CALL_FLAG_IGNORE_RECLIMIT causes duk_handle_call() to ignore C
	 *  recursion depth limit (and won't increase it either).  This is
	 *  dangerous, but useful because it allows an errhandler to run even
	 *  if the original error is caused by C recursion depth limit.
	 *
	 *  The heap level DUK_HEAP_FLAG_ERRHANDLER_RUNNING is set for the
	 *  duration of the errhandler call and cleared afterwards.  This
	 *  flag prevents the error handler from running recursively.  The
	 *  flag is heap level so that the flag properly controls even coroutines
	 *  launched by an error handler.  Since the flag is heap level, it is
	 *  critical to restore it correctly.
	 *
	 *  We ignore errors now: a success return and an error value both
	 *  replace the original error value.  (This would be easy to change.)
	 */

	DUK_ASSERT(!DUK_HEAP_HAS_ERRHANDLER_RUNNING(thr->heap));  /* since no recursive errhandlers */
	DUK_HEAP_SET_ERRHANDLER_RUNNING(thr->heap);

	call_flags = DUK_CALL_FLAG_PROTECTED |
	             DUK_CALL_FLAG_IGNORE_RECLIMIT;  /* protected, ignore reclimit, not constructor */

	rc = duk_handle_call(thr,
	                     1,            /* num args */
	                     call_flags);  /* call_flags */
	DUK_UNREF(rc);  /* no need to check now: both success and error are OK */

	DUK_ASSERT(DUK_HEAP_HAS_ERRHANDLER_RUNNING(thr->heap));
	DUK_HEAP_CLEAR_ERRHANDLER_RUNNING(thr->heap);

	/* [ ... errval ] */
}

/*
 *  Create and throw an error
 *
 *  Push an error object on top of the stack, possibly call an errhandler,
 *  and finally longjmp.
 *
 *  If an error occurs while we're dealing with the current error, we might
 *  enter an infinite recursion loop.  This is prevented by detecting a
 *  "double fault" through the heap->handling_error flag; the recursion
 *  then stops at the second level.
 */

#ifdef DUK_USE_VERBOSE_ERRORS
void duk_err_create_and_throw(duk_hthread *thr, duk_uint32_t code, const char *msg, const char *filename, int line) {
#else
void duk_err_create_and_throw(duk_hthread *thr, duk_uint32_t code) {
#endif
	duk_context *ctx = (duk_context *) thr;
	int double_error = thr->heap->handling_error;

#ifdef DUK_USE_VERBOSE_ERRORS
	DUK_DDPRINT("duk_err_create_and_throw(): code=%d, msg=%s, filename=%s, line=%d",
	             code, msg ? msg : "null", filename ? filename : "null", line);
#else
	DUK_DDPRINT("duk_err_create_and_throw(): code=%d", code);
#endif

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);

	thr->heap->handling_error = 1;

	/*
	 *  Create and push an error object onto the top of stack.
	 *  If a "double error" occurs, use a fixed error instance
	 *  to avoid further trouble.
	 */

	/* FIXME: if attempt to push beyond allocated valstack, this double fault
	 * handling fails miserably.  We should really write the double error
	 * directly to thr->heap->lj.value1 and avoid valstack use entirely.
	 */

	if (double_error) {
		if (thr->builtins[DUK_BIDX_DOUBLE_ERROR]) {
			DUK_DPRINT("double fault detected -> push built-in fixed 'double error' instance");
			duk_push_hobject(ctx, thr->builtins[DUK_BIDX_DOUBLE_ERROR]);
		} else {
			DUK_DPRINT("double fault detected; there is no built-in fixed 'double error' instance "
			           "-> push the error code as a number");
			duk_push_int(ctx, code);
		}
	} else {
		/* Error object is augmented at its creation here. */
		duk_require_stack(ctx, 1);
		/* FIXME: unnecessary '%s' formatting here */
#ifdef DUK_USE_VERBOSE_ERRORS
		duk_push_error_object_raw(ctx,
		                          code | DUK_ERRCODE_FLAG_NOBLAME_FILELINE,
		                          filename,
		                          line,
		                          "%s",
		                          msg);
#else
		duk_push_error_object_raw(ctx,
		                          code | DUK_ERRCODE_FLAG_NOBLAME_FILELINE,
		                          NULL,
		                          0,
		                          NULL);
#endif
	}

	/*
	 *  Call errhandler (unless error is an alloc error)
	 */

	if (double_error || code == DUK_ERR_ALLOC_ERROR) {
		DUK_DPRINT("alloc or double error: skip calling errhandler to avoid further trouble");
	} else {
		DUK_DDDPRINT("THROW ERROR (INTERNAL): %!iT (before errhandler)", duk_get_tval(ctx, -1));
		duk_err_call_errhandler(thr);
	}

	/*
	 *  Finally, longjmp
	 */

	thr->heap->handling_error = 0;

	duk_err_setup_heap_ljstate(thr, DUK_LJ_TYPE_THROW);

	DUK_DDDPRINT("THROW ERROR (INTERNAL): %!iT, %!iT (after errhandler)",
	             &thr->heap->lj.value1, &thr->heap->lj.value2);

	duk_err_longjmp(thr);
	DUK_UNREACHABLE();
}

/*
 *  Helper for C function call negative return values.
 */

void duk_error_throw_from_negative_rc(duk_hthread *thr, int rc) {
	duk_context *ctx = (duk_context *) thr;
	const char *msg;
	int code;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(rc < 0);

	/* FIXME: this generates quite large code - perhaps select the error
	 * class based on the code and then just use the error 'name'?
	 */

	code = -rc;

	switch (rc) {
	case DUK_RET_UNIMPLEMENTED_ERROR:  msg = "unimplemented"; break;
	case DUK_RET_UNSUPPORTED_ERROR:    msg = "unsupported"; break;
	case DUK_RET_INTERNAL_ERROR:       msg = "internal"; break;
	case DUK_RET_ALLOC_ERROR:          msg = "alloc"; break;
	case DUK_RET_ASSERTION_ERROR:      msg = "assertion"; break;
	case DUK_RET_API_ERROR:            msg = "api"; break;
	case DUK_RET_UNCAUGHT_ERROR:       msg = "uncaught"; break;
	case DUK_RET_ERROR:                msg = "error"; break;
	case DUK_RET_EVAL_ERROR:           msg = "eval"; break;
	case DUK_RET_RANGE_ERROR:          msg = "range"; break;
	case DUK_RET_REFERENCE_ERROR:      msg = "reference"; break;
	case DUK_RET_SYNTAX_ERROR:         msg = "syntax"; break;
	case DUK_RET_TYPE_ERROR:           msg = "type"; break;
	case DUK_RET_URI_ERROR:            msg = "uri"; break;
	default:                           msg = "unknown"; break;
	}

	DUK_ASSERT(msg != NULL);

	/*
	 *  The __FILE__ and __LINE__ information is intentionally not used in the
	 *  creation of the error object, as it isn't useful in the tracedata.  The
	 *  tracedata still contains the function which returned the negative return
	 *  code, and having the file/line of this function isn't very useful.
	 */

	duk_error_raw(ctx, code, NULL, 0, "%s error (rc %d)", msg, rc);
	DUK_UNREACHABLE();
}
