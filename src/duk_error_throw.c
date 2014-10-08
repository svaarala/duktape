/*
 *  Create and throw an Ecmascript error object based on a code and a message.
 *
 *  Used when we throw errors internally.  Ecmascript generated error objects
 *  are created by Ecmascript code, and the throwing is handled by the bytecode
 *  executor.
 */

#include "duk_internal.h"

/*
 *  Create and throw an error (originating from Duktape internally)
 *
 *  Push an error object on top of the stack, possibly throw augmenting
 *  the error, and finally longjmp.
 *
 *  If an error occurs while we're dealing with the current error, we might
 *  enter an infinite recursion loop.  This is prevented by detecting a
 *  "double fault" through the heap->handling_error flag; the recursion
 *  then stops at the second level.
 */

#ifdef DUK_USE_VERBOSE_ERRORS
DUK_INTERNAL void duk_err_create_and_throw(duk_hthread *thr, duk_errcode_t code, const char *msg, const char *filename, duk_int_t line) {
#else
DUK_INTERNAL void duk_err_create_and_throw(duk_hthread *thr, duk_errcode_t code) {
#endif
	duk_context *ctx = (duk_context *) thr;
	duk_bool_t double_error = thr->heap->handling_error;

#ifdef DUK_USE_VERBOSE_ERRORS
	DUK_DD(DUK_DDPRINT("duk_err_create_and_throw(): code=%ld, msg=%s, filename=%s, line=%ld",
	                   (long) code, (const char *) msg,
	                   (const char *) filename, (long) line));
#else
	DUK_DD(DUK_DDPRINT("duk_err_create_and_throw(): code=%ld", (long) code));
#endif

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);

	thr->heap->handling_error = 1;

	/*
	 *  Create and push an error object onto the top of stack.
	 *  If a "double error" occurs, use a fixed error instance
	 *  to avoid further trouble.
	 */

	/* XXX: if attempt to push beyond allocated valstack, this double fault
	 * handling fails miserably.  We should really write the double error
	 * directly to thr->heap->lj.value1 and avoid valstack use entirely.
	 */

	if (double_error) {
		if (thr->builtins[DUK_BIDX_DOUBLE_ERROR]) {
			DUK_D(DUK_DPRINT("double fault detected -> push built-in fixed 'double error' instance"));
			duk_push_hobject_bidx(ctx, DUK_BIDX_DOUBLE_ERROR);
		} else {
			DUK_D(DUK_DPRINT("double fault detected; there is no built-in fixed 'double error' instance "
			                 "-> push the error code as a number"));
			duk_push_int(ctx, (duk_int_t) code);
		}
	} else {
		/* Error object is augmented at its creation here. */
		duk_require_stack(ctx, 1);
		/* XXX: unnecessary '%s' formatting here, but cannot use
		 * 'msg' as a format string directly.
		 */
#ifdef DUK_USE_VERBOSE_ERRORS
		duk_push_error_object_raw(ctx,
		                          code | DUK_ERRCODE_FLAG_NOBLAME_FILELINE,
		                          filename,
		                          line,
		                          "%s",
		                          (const char *) msg);
#else
		duk_push_error_object_raw(ctx,
		                          code | DUK_ERRCODE_FLAG_NOBLAME_FILELINE,
		                          NULL,
		                          0,
		                          NULL);
#endif
	}

	/*
	 *  Augment error (throw time), unless alloc/double error
	 */

	if (double_error || code == DUK_ERR_ALLOC_ERROR) {
		DUK_D(DUK_DPRINT("alloc or double error: skip throw augmenting to avoid further trouble"));
	} else {
#if defined(DUK_USE_AUGMENT_ERROR_THROW)
		DUK_DDD(DUK_DDDPRINT("THROW ERROR (INTERNAL): %!iT (before throw augment)",
		                     (duk_tval *) duk_get_tval(ctx, -1)));
		duk_err_augment_error_throw(thr);
#endif
	}

	/*
	 *  Finally, longjmp
	 */

	thr->heap->handling_error = 0;

	duk_err_setup_heap_ljstate(thr, DUK_LJ_TYPE_THROW);

	DUK_DDD(DUK_DDDPRINT("THROW ERROR (INTERNAL): %!iT, %!iT (after throw augment)",
	                     (duk_tval *) &thr->heap->lj.value1, (duk_tval *) &thr->heap->lj.value2));

	duk_err_longjmp(thr);
	DUK_UNREACHABLE();
}

/*
 *  Helper for C function call negative return values.
 */

DUK_INTERNAL void duk_error_throw_from_negative_rc(duk_hthread *thr, duk_ret_t rc) {
	duk_context *ctx = (duk_context *) thr;
	const char *msg;
	duk_errcode_t code;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(rc < 0);

	/* XXX: this generates quite large code - perhaps select the error
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

	duk_error_raw(ctx, code, NULL, 0, "%s error (rc %ld)", (const char *) msg, (long) rc);
	DUK_UNREACHABLE();
}
