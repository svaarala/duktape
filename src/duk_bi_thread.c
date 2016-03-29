/*
 *  Thread builtins
 */

#include "duk_internal.h"

/*
 *  Constructor
 */

DUK_INTERNAL duk_ret_t duk_bi_thread_constructor(duk_context *ctx) {
	duk_hthread *new_thr;
	duk_hobject *func;

	/* XXX: need a duk_require_func_or_lfunc_coerce() */
	if (!duk_is_callable(ctx, 0)) {
		return DUK_RET_TYPE_ERROR;
	}
	func = duk_require_hobject_or_lfunc_coerce(ctx, 0);
	DUK_ASSERT(func != NULL);

	duk_push_thread(ctx);
	new_thr = (duk_hthread *) duk_get_hobject(ctx, -1);
	DUK_ASSERT(new_thr != NULL);
	new_thr->state = DUK_HTHREAD_STATE_INACTIVE;

	/* push initial function call to new thread stack; this is
	 * picked up by resume().
	 */
	duk_push_hobject((duk_context *) new_thr, func);

	return 1;  /* return thread */
}

/*
 *  Resume a thread.
 *
 *  The thread must be in resumable state, either (a) new thread which hasn't
 *  yet started, or (b) a thread which has previously yielded.  This method
 *  must be called from an Ecmascript function.
 *
 *  Args:
 *    - thread
 *    - value
 *    - isError (defaults to false)
 *
 *  Note: yield and resume handling is currently asymmetric.
 */

DUK_INTERNAL duk_ret_t duk_bi_thread_resume(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hthread *thr_resume;
	duk_tval *tv;
	duk_hobject *func;
	duk_hobject *caller_func;
	duk_small_int_t is_error;

	DUK_DDD(DUK_DDDPRINT("Duktape.Thread.resume(): thread=%!T, value=%!T, is_error=%!T",
	                     (duk_tval *) duk_get_tval(ctx, 0),
	                     (duk_tval *) duk_get_tval(ctx, 1),
	                     (duk_tval *) duk_get_tval(ctx, 2)));

	DUK_ASSERT(thr->state == DUK_HTHREAD_STATE_RUNNING);
	DUK_ASSERT(thr->heap->curr_thread == thr);

	thr_resume = duk_require_hthread(ctx, 0);
	is_error = (duk_small_int_t) duk_to_boolean(ctx, 2);
	duk_set_top(ctx, 2);

	/* [ thread value ] */

	/*
	 *  Thread state and calling context checks
	 */

	if (thr->callstack_top < 2) {
		DUK_DD(DUK_DDPRINT("resume state invalid: callstack should contain at least 2 entries (caller and Duktape.Thread.resume)"));
		goto state_error;
	}
	DUK_ASSERT(DUK_ACT_GET_FUNC(thr->callstack + thr->callstack_top - 1) != NULL);  /* us */
	DUK_ASSERT(DUK_HOBJECT_IS_NATIVEFUNCTION(DUK_ACT_GET_FUNC(thr->callstack + thr->callstack_top - 1)));
	DUK_ASSERT(DUK_ACT_GET_FUNC(thr->callstack + thr->callstack_top - 2) != NULL);  /* caller */

	caller_func = DUK_ACT_GET_FUNC(thr->callstack + thr->callstack_top - 2);
	if (!DUK_HOBJECT_IS_COMPILEDFUNCTION(caller_func)) {
		DUK_DD(DUK_DDPRINT("resume state invalid: caller must be Ecmascript code"));
		goto state_error;
	}

	/* Note: there is no requirement that: 'thr->callstack_preventcount == 1'
	 * like for yield.
	 */

	if (thr_resume->state != DUK_HTHREAD_STATE_INACTIVE &&
	    thr_resume->state != DUK_HTHREAD_STATE_YIELDED) {
		DUK_DD(DUK_DDPRINT("resume state invalid: target thread must be INACTIVE or YIELDED"));
		goto state_error;
	}

	DUK_ASSERT(thr_resume->state == DUK_HTHREAD_STATE_INACTIVE ||
	           thr_resume->state == DUK_HTHREAD_STATE_YIELDED);

	/* Further state-dependent pre-checks */

	if (thr_resume->state == DUK_HTHREAD_STATE_YIELDED) {
		/* no pre-checks now, assume a previous yield() has left things in
		 * tip-top shape (longjmp handler will assert for these).
		 */
	} else {
		DUK_ASSERT(thr_resume->state == DUK_HTHREAD_STATE_INACTIVE);

		if ((thr_resume->callstack_top != 0) ||
		    (thr_resume->valstack_top - thr_resume->valstack != 1)) {
			goto state_invalid_initial;
		}
		tv = &thr_resume->valstack_top[-1];
		DUK_ASSERT(tv >= thr_resume->valstack && tv < thr_resume->valstack_top);
		if (!DUK_TVAL_IS_OBJECT(tv)) {
			goto state_invalid_initial;
		}
		func = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(func != NULL);
		if (!DUK_HOBJECT_IS_COMPILEDFUNCTION(func)) {
			/* Note: cannot be a bound function either right now,
			 * this would be easy to relax though.
			 */
			goto state_invalid_initial;
		}

	}

	/*
	 *  The error object has been augmented with a traceback and other
	 *  info from its creation point -- usually another thread.  The
	 *  error handler is called here right before throwing, but it also
	 *  runs in the resumer's thread.  It might be nice to get a traceback
	 *  from the resumee but this is not the case now.
	 */

#if defined(DUK_USE_AUGMENT_ERROR_THROW)
	if (is_error) {
		DUK_ASSERT_TOP(ctx, 2);  /* value (error) is at stack top */
		duk_err_augment_error_throw(thr);  /* in resumer's context */
	}
#endif

#ifdef DUK_USE_DEBUG
	if (is_error) {
		DUK_DDD(DUK_DDDPRINT("RESUME ERROR: thread=%!T, value=%!T",
		                     (duk_tval *) duk_get_tval(ctx, 0),
		                     (duk_tval *) duk_get_tval(ctx, 1)));
	} else if (thr_resume->state == DUK_HTHREAD_STATE_YIELDED) {
		DUK_DDD(DUK_DDDPRINT("RESUME NORMAL: thread=%!T, value=%!T",
		                     (duk_tval *) duk_get_tval(ctx, 0),
		                     (duk_tval *) duk_get_tval(ctx, 1)));
	} else {
		DUK_DDD(DUK_DDDPRINT("RESUME INITIAL: thread=%!T, value=%!T",
		                     (duk_tval *) duk_get_tval(ctx, 0),
		                     (duk_tval *) duk_get_tval(ctx, 1)));
	}
#endif

	thr->heap->lj.type = DUK_LJ_TYPE_RESUME;

	/* lj value2: thread */
	DUK_ASSERT(thr->valstack_bottom < thr->valstack_top);
	DUK_TVAL_SET_TVAL_UPDREF(thr, &thr->heap->lj.value2, &thr->valstack_bottom[0]);  /* side effects */

	/* lj value1: value */
	DUK_ASSERT(thr->valstack_bottom + 1 < thr->valstack_top);
	DUK_TVAL_SET_TVAL_UPDREF(thr, &thr->heap->lj.value1, &thr->valstack_bottom[1]);  /* side effects */
	DUK_TVAL_CHKFAST_INPLACE(&thr->heap->lj.value1);

	thr->heap->lj.iserror = is_error;

	DUK_ASSERT(thr->heap->lj.jmpbuf_ptr != NULL);  /* call is from executor, so we know we have a jmpbuf */
	duk_err_longjmp(thr);  /* execution resumes in bytecode executor */
	return 0;  /* never here */

 state_invalid_initial:
	DUK_ERROR_TYPE(thr, "invalid initial thread state/stack");
	return 0;  /* never here */

 state_error:
	DUK_ERROR_TYPE(thr, "invalid state");
	return 0;  /* never here */
}

/*
 *  Yield the current thread.
 *
 *  The thread must be in yieldable state: it must have a resumer, and there
 *  must not be any yield-preventing calls (native calls and constructor calls,
 *  currently) in the thread's call stack (otherwise a resume would not be
 *  possible later).  This method must be called from an Ecmascript function.
 *
 *  Args:
 *    - value
 *    - isError (defaults to false)
 *
 *  Note: yield and resume handling is currently asymmetric.
 */

DUK_INTERNAL duk_ret_t duk_bi_thread_yield(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *caller_func;
	duk_small_int_t is_error;

	DUK_DDD(DUK_DDDPRINT("Duktape.Thread.yield(): value=%!T, is_error=%!T",
	                     (duk_tval *) duk_get_tval(ctx, 0),
	                     (duk_tval *) duk_get_tval(ctx, 1)));

	DUK_ASSERT(thr->state == DUK_HTHREAD_STATE_RUNNING);
	DUK_ASSERT(thr->heap->curr_thread == thr);

	is_error = (duk_small_int_t) duk_to_boolean(ctx, 1);
	duk_set_top(ctx, 1);

	/* [ value ] */

	/*
	 *  Thread state and calling context checks
	 */

	if (!thr->resumer) {
		DUK_DD(DUK_DDPRINT("yield state invalid: current thread must have a resumer"));
		goto state_error;
	}
	DUK_ASSERT(thr->resumer->state == DUK_HTHREAD_STATE_RESUMED);

	if (thr->callstack_top < 2) {
		DUK_DD(DUK_DDPRINT("yield state invalid: callstack should contain at least 2 entries (caller and Duktape.Thread.yield)"));
		goto state_error;
	}
	DUK_ASSERT(DUK_ACT_GET_FUNC(thr->callstack + thr->callstack_top - 1) != NULL);  /* us */
	DUK_ASSERT(DUK_HOBJECT_IS_NATIVEFUNCTION(DUK_ACT_GET_FUNC(thr->callstack + thr->callstack_top - 1)));
	DUK_ASSERT(DUK_ACT_GET_FUNC(thr->callstack + thr->callstack_top - 2) != NULL);  /* caller */

	caller_func = DUK_ACT_GET_FUNC(thr->callstack + thr->callstack_top - 2);
	if (!DUK_HOBJECT_IS_COMPILEDFUNCTION(caller_func)) {
		DUK_DD(DUK_DDPRINT("yield state invalid: caller must be Ecmascript code"));
		goto state_error;
	}

	DUK_ASSERT(thr->callstack_preventcount >= 1);  /* should never be zero, because we (Duktape.Thread.yield) are on the stack */
	if (thr->callstack_preventcount != 1) {
		/* Note: the only yield-preventing call is Duktape.Thread.yield(), hence check for 1, not 0 */
		DUK_DD(DUK_DDPRINT("yield state invalid: there must be no yield-preventing calls in current thread callstack (preventcount is %ld)",
		                   (long) thr->callstack_preventcount));
		goto state_error;
	}

	/*
	 *  The error object has been augmented with a traceback and other
	 *  info from its creation point -- usually the current thread.
	 *  The error handler, however, is called right before throwing
	 *  and runs in the yielder's thread.
	 */

#if defined(DUK_USE_AUGMENT_ERROR_THROW)
	if (is_error) {
		DUK_ASSERT_TOP(ctx, 1);  /* value (error) is at stack top */
		duk_err_augment_error_throw(thr);  /* in yielder's context */
	}
#endif

#ifdef DUK_USE_DEBUG
	if (is_error) {
		DUK_DDD(DUK_DDDPRINT("YIELD ERROR: value=%!T",
		                     (duk_tval *) duk_get_tval(ctx, 0)));
	} else {
		DUK_DDD(DUK_DDDPRINT("YIELD NORMAL: value=%!T",
		                     (duk_tval *) duk_get_tval(ctx, 0)));
	}
#endif

	/*
	 *  Process yield
	 *
	 *  After longjmp(), processing continues in bytecode executor longjmp
	 *  handler, which will e.g. update thr->resumer to NULL.
	 */

	thr->heap->lj.type = DUK_LJ_TYPE_YIELD;

	/* lj value1: value */
	DUK_ASSERT(thr->valstack_bottom < thr->valstack_top);
	DUK_TVAL_SET_TVAL_UPDREF(thr, &thr->heap->lj.value1, &thr->valstack_bottom[0]);  /* side effects */
	DUK_TVAL_CHKFAST_INPLACE(&thr->heap->lj.value1);

	thr->heap->lj.iserror = is_error;

	DUK_ASSERT(thr->heap->lj.jmpbuf_ptr != NULL);  /* call is from executor, so we know we have a jmpbuf */
	duk_err_longjmp(thr);  /* execution resumes in bytecode executor */
	return 0;  /* never here */

 state_error:
	DUK_ERROR_TYPE(thr, "invalid state");
	return 0;  /* never here */
}

DUK_INTERNAL duk_ret_t duk_bi_thread_current(duk_context *ctx) {
	duk_push_current_thread(ctx);
	return 1;
}
