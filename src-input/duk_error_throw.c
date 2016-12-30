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

#if defined(DUK_USE_VERBOSE_ERRORS)
DUK_INTERNAL void duk_err_create_and_throw(duk_hthread *thr, duk_errcode_t code, const char *msg, const char *filename, duk_int_t line) {
#else
DUK_INTERNAL void duk_err_create_and_throw(duk_hthread *thr, duk_errcode_t code) {
#endif
	duk_context *ctx = (duk_context *) thr;
	duk_bool_t double_error = thr->heap->handling_error;

#if defined(DUK_USE_VERBOSE_ERRORS)
	DUK_DD(DUK_DDPRINT("duk_err_create_and_throw(): code=%ld, msg=%s, filename=%s, line=%ld",
	                   (long) code, (const char *) msg,
	                   (const char *) filename, (long) line));
#else
	DUK_DD(DUK_DDPRINT("duk_err_create_and_throw(): code=%ld", (long) code));
#endif

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);

	thr->heap->handling_error = 1;

	if (!double_error) {
		/* Allow headroom for calls during error handling (see GH-191).
		 * We allow space for 10 additional recursions, with one extra
		 * for, e.g. a print() call at the deepest level.
		 */
		DUK_ASSERT(thr->callstack_max == DUK_CALLSTACK_DEFAULT_MAX);
		thr->callstack_max = DUK_CALLSTACK_DEFAULT_MAX + DUK_CALLSTACK_GROW_STEP + 11;
	}

	DUK_ASSERT(thr->callstack_max == DUK_CALLSTACK_DEFAULT_MAX + DUK_CALLSTACK_GROW_STEP + 11);  /* just making sure */

	/* Sync so that augmentation sees up-to-date activations, NULL
	 * thr->ptr_curr_pc so that it's not used if side effects occur
	 * in augmentation or longjmp handling.
	 */
	duk_hthread_sync_and_null_currpc(thr);

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
#if defined(DUK_USE_VERBOSE_ERRORS)
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
	 *  Augment error (throw time), unless double error
	 *
	 *  Note that an alloc error may happen during error augmentation.
	 *  This may happen both when the original error is an alloc error
	 *  and when it's something else.  Because any error in augmentation
	 *  must be handled correctly anyway, there's no special check for
	 *  avoiding it for alloc errors (this differs from Duktape 1.x).
	 */

	if (double_error) {
		DUK_D(DUK_DPRINT("double error: skip throw augmenting to avoid further trouble"));
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

	duk_err_setup_heap_ljstate(thr, DUK_LJ_TYPE_THROW);

	thr->callstack_max = DUK_CALLSTACK_DEFAULT_MAX;  /* reset callstack limit */
	thr->heap->handling_error = 0;

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

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(rc < 0);

	/*
	 *  The __FILE__ and __LINE__ information is intentionally not used in the
	 *  creation of the error object, as it isn't useful in the tracedata.  The
	 *  tracedata still contains the function which returned the negative return
	 *  code, and having the file/line of this function isn't very useful.
	 *
	 *  The error messages for DUK_RET_xxx shorthand are intentionally very
	 *  minimal: they're only really useful for low memory targets.
	 */

	duk_error_raw(ctx, -rc, NULL, 0, "error (rc %ld)", (long) rc);
	DUK_UNREACHABLE();
}
