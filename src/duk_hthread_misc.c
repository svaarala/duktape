/*
 *  Thread support.
 */

#include "duk_internal.h"

/* FIXME: separate "executor" thread and "target" thread for DECREF?
 * This function is a bit dangerous because we free the built-ins and
 * DECREF.  If a built-in gets DECREF'd to zero and has a finalizer,
 * we might have some problems in the finalizer.
 */

void duk_hthread_terminate(duk_hthread *thr) {
	int i;

	duk_hthread_callstack_unwind(thr, 0);  /* side effects, possibly errors */

	duk_hthread_catchstack_unwind(thr, 0);

	thr->valstack_bottom = thr->valstack;
	duk_set_top((duk_context *) thr, 0);  /* unwinds valstack, updating refcounts */

	for (i = 0; i < DUK_NUM_BUILTINS; i++) {
#ifdef DUK_USE_REFERENCE_COUNTING
		duk_hobject *h = thr->builtins[i];
#endif
		thr->builtins[i] = NULL;
#ifdef DUK_USE_REFERENCE_COUNTING
		DUK_HOBJECT_DECREF(thr, h);
#endif
	}

	thr->state = DUK_HTHREAD_STATE_TERMINATED;

	/* FIXME: shrink to minimum size, but don't free stacks? */
}


