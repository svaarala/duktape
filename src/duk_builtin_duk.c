/*
 *  __duk__ built-ins
 */

#include <sys/time.h>

#include "duk_internal.h"

int duk_builtin_duk_object_addr(duk_context *ctx) {
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

int duk_builtin_duk_object_refc(duk_context *ctx) {
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

int duk_builtin_duk_object_gc(duk_context *ctx) {
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

int duk_builtin_duk_object_get_finalizer(duk_context *ctx) {
	(void) duk_require_hobject(ctx, 0);
	duk_get_prop_stridx(ctx, 0, DUK_STRIDX_INT_FINALIZER);
	return 1;
}

int duk_builtin_duk_object_set_finalizer(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 2);
	(void) duk_put_prop_stridx(ctx, 0, DUK_STRIDX_INT_FINALIZER);  /* XXX: check value? */
	return 0;
}

/*
 *  Spawn a thread.
 */

int duk_builtin_duk_object_spawn(duk_context *ctx) {
	duk_hthread *new_thr;
	duk_hobject *func;

	if (!duk_is_callable(ctx, 0)) {
		return DUK_RET_TYPE_ERROR;
	}
	func = duk_get_hobject(ctx, 0);
	DUK_ASSERT(func != NULL);

	duk_push_new_thread(ctx);
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

int duk_builtin_duk_object_resume(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hthread *thr_resume;
	duk_tval tv_tmp;
	duk_tval *tv;
	duk_hobject *func;
	int is_error;

	DUK_DDDPRINT("__duk__.resume(): thread=%!T, value=%!T, is_error=%!T",
	             duk_get_tval(ctx, 0),
	             duk_get_tval(ctx, 1),
	             duk_get_tval(ctx, 2));

	DUK_ASSERT(thr->state == DUK_HTHREAD_STATE_RUNNING);
	DUK_ASSERT(thr->heap->curr_thread == thr);

	thr_resume = duk_require_hthread(ctx, 0);
	is_error = duk_to_boolean(ctx, 2);

	/*
	 *  Thread state and calling context checks
	 */

	if (thr->callstack_top < 2) {
		DUK_DDPRINT("resume state invalid: callstack should contain at least 2 entries (caller and __duk__.resume)");
		goto state_error;
	}
	DUK_ASSERT((thr->callstack + thr->callstack_top - 1)->func != NULL);  /* us */
	DUK_ASSERT(DUK_HOBJECT_IS_NATIVEFUNCTION((thr->callstack + thr->callstack_top - 1)->func));
	DUK_ASSERT((thr->callstack + thr->callstack_top - 2)->func != NULL);  /* caller */

	if (!DUK_HOBJECT_IS_COMPILEDFUNCTION((thr->callstack + thr->callstack_top - 2)->func)) {
		DUK_DDPRINT("resume state invalid: caller must be Ecmascript code");
		goto state_error;
	}

	/* Note: there is no requirement that: 'thr->callstack_preventcount == 1'
	 * like for yield.
	 */

	if (thr_resume->state != DUK_HTHREAD_STATE_INACTIVE &&
	    thr_resume->state != DUK_HTHREAD_STATE_YIELDED) {
		DUK_DDPRINT("resume state invalid: target thread must be INACTIVE or YIELDED");
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
	 *  info from its creation point -- usually another thread.  It might
	 *  be nice to get a traceback from the resumee but this is not the
	 *  case now.
	 */

#ifdef DUK_USE_DEBUG  /* debug logging */
	if (is_error) {
		DUK_DDDPRINT("RESUME ERROR: thread=%!T, value=%!T",
		             duk_get_tval(ctx, 0),
		             duk_get_tval(ctx, 1));
	} else if (thr_resume->state == DUK_HTHREAD_STATE_YIELDED) {
		DUK_DDDPRINT("RESUME NORMAL: thread=%!T, value=%!T",
		             duk_get_tval(ctx, 0),
		             duk_get_tval(ctx, 1));
	} else {
		DUK_DDDPRINT("RESUME INITIAL: thread=%!T, value=%!T",
		             duk_get_tval(ctx, 0),
		             duk_get_tval(ctx, 1));
	}
#endif

	thr->heap->lj.type = DUK_LJ_TYPE_RESUME;

	/* lj value2: thread */
	DUK_ASSERT(thr->valstack_bottom < thr->valstack_top);
	DUK_TVAL_SET_TVAL(&tv_tmp, &thr->heap->lj.value2);
	DUK_TVAL_SET_TVAL(&thr->heap->lj.value2, &thr->valstack_bottom[0]);
	DUK_TVAL_INCREF(thr, &thr->heap->lj.value2);
	DUK_TVAL_DECREF(thr, &tv_tmp);

	/* lj value1: value */
	DUK_ASSERT(thr->valstack_bottom + 1 < thr->valstack_top);
	DUK_TVAL_SET_TVAL(&tv_tmp, &thr->heap->lj.value1);
	DUK_TVAL_SET_TVAL(&thr->heap->lj.value1, &thr->valstack_bottom[1]);
	DUK_TVAL_INCREF(thr, &thr->heap->lj.value1);
	DUK_TVAL_DECREF(thr, &tv_tmp);

	thr->heap->lj.iserror = is_error;

	DUK_ASSERT(thr->heap->lj.jmpbuf_ptr != NULL);  /* call is from executor, so we know we have a jmpbuf */
	duk_err_longjmp(thr);  /* execution resumes in bytecode executor */
	return 0;  /* never here */

 state_invalid_initial:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "invalid initial thread state/stack");
	return 0;  /* never here */

 state_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "invalid state for resume");
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

int duk_builtin_duk_object_yield(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval tv_tmp;
	int is_error;

	DUK_DDDPRINT("__duk__.yield(): value=%!T, is_error=%!T",
	             duk_get_tval(ctx, 0),
	             duk_get_tval(ctx, 1));

	DUK_ASSERT(thr->state == DUK_HTHREAD_STATE_RUNNING);
	DUK_ASSERT(thr->heap->curr_thread == thr);

	is_error = duk_to_boolean(ctx, 1);

	/*
	 *  Thread state and calling context checks
	 */

	if (!thr->resumer) {
		DUK_DDPRINT("yield state invalid: current thread must have a resumer");
		goto state_error;
	}
	DUK_ASSERT(thr->resumer->state == DUK_HTHREAD_STATE_RESUMED);

	if (thr->callstack_top < 2) {
		DUK_DDPRINT("yield state invalid: callstack should contain at least 2 entries (caller and __duk__.yield)");
		goto state_error;
	}
	DUK_ASSERT((thr->callstack + thr->callstack_top - 1)->func != NULL);  /* us */
	DUK_ASSERT(DUK_HOBJECT_IS_NATIVEFUNCTION((thr->callstack + thr->callstack_top - 1)->func));
	DUK_ASSERT((thr->callstack + thr->callstack_top - 2)->func != NULL);  /* caller */

	if (!DUK_HOBJECT_IS_COMPILEDFUNCTION((thr->callstack + thr->callstack_top - 2)->func)) {
		DUK_DDPRINT("yield state invalid: caller must be Ecmascript code");
		goto state_error;
	}

	DUK_ASSERT(thr->callstack_preventcount >= 1);  /* should never be zero, because we (__duk__.yield) are on the stack */
	if (thr->callstack_preventcount != 1) {
		/* Note: the only yield-preventing call is __duk__.yield(), hence check for 1, not 0 */
		DUK_DDPRINT("yield state invalid: there must be no yield-preventing calls in current thread callstack (preventcount is %d)",
		            thr->callstack_preventcount);
		goto state_error;
	}

	/*
	 *  The error object has been augmented with a traceback and other
	 *  info from its creation point -- usually the current thread.
	 */

#ifdef DUK_USE_DEBUG
	if (is_error) {
		DUK_DDDPRINT("YIELD ERROR: value=%!T",
		             duk_get_tval(ctx, 0));
	} else {
		DUK_DDDPRINT("YIELD NORMAL: value=%!T",
		             duk_get_tval(ctx, 0));
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
	DUK_TVAL_SET_TVAL(&tv_tmp, &thr->heap->lj.value1);
	DUK_TVAL_SET_TVAL(&thr->heap->lj.value1, &thr->valstack_bottom[0]);
	DUK_TVAL_INCREF(thr, &thr->heap->lj.value1);
	DUK_TVAL_DECREF(thr, &tv_tmp);

	thr->heap->lj.iserror = is_error;

	DUK_ASSERT(thr->heap->lj.jmpbuf_ptr != NULL);  /* call is from executor, so we know we have a jmpbuf */
	duk_err_longjmp(thr);  /* execution resumes in bytecode executor */
	return 0;  /* never here */

 state_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "invalid state for yield");
	return 0;  /* never here */
}

int duk_builtin_duk_object_curr(duk_context *ctx) {
	duk_push_current_thread(ctx);
	return 1;
}

int duk_builtin_duk_object_print(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_duk_object_time(duk_context *ctx) {
	struct timeval tv;

	memset(&tv, 0, sizeof(tv));
	if (gettimeofday(&tv, NULL) != 0) {
		return DUK_RET_ERROR;
	}
	duk_push_number(ctx, (double) tv.tv_sec + ((double) tv.tv_usec) / 1000000.0);
	return 1;
}

int duk_builtin_duk_object_enc(duk_context *ctx) {
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

int duk_builtin_duk_object_dec(duk_context *ctx) {
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

int duk_builtin_duk_object_sleep(duk_context *ctx) {
	struct timeval tv;
	int msec;

	/* FIXME: signal handling */
	msec = duk_to_int(ctx, 0);
	tv.tv_sec = msec / 1000;
	tv.tv_usec = (msec % 1000) * 1000;
	(void) select(0, NULL, NULL, NULL, &tv);
	return 0;
}

