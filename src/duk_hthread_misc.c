/*
 *  Thread support.
 */

#include "duk_internal.h"

DUK_INTERNAL void duk_hthread_terminate(duk_hthread *thr) {
	DUK_ASSERT(thr != NULL);

	/* Order of unwinding is important */

	duk_hthread_catchstack_unwind(thr, 0);

	duk_hthread_callstack_unwind(thr, 0);  /* side effects, possibly errors */

	thr->valstack_bottom = thr->valstack;
	duk_set_top((duk_context *) thr, 0);  /* unwinds valstack, updating refcounts */

	thr->state = DUK_HTHREAD_STATE_TERMINATED;

	/* Here we could remove references to built-ins, but it may not be
	 * worth the effort because built-ins are quite likely to be shared
	 * with another (unterminated) thread, and terminated threads are also
	 * usually garbage collected quite quickly.  Also, doing DECREFs
	 * could trigger finalization, which would run on the current thread
	 * and have access to only some of the built-ins.  Garbage collection
	 * deals with this correctly already.
	 */

	/* XXX: Shrink the stacks to minimize memory usage?  May not
	 * be worth the effort because terminated threads are usually
	 * garbage collected quite soon.
	 */
}

DUK_INTERNAL duk_activation *duk_hthread_get_current_activation(duk_hthread *thr) {
	DUK_ASSERT(thr != NULL);

	if (thr->callstack_top > 0) {
		return thr->callstack + thr->callstack_top - 1;
	} else {
		return NULL;
	}
}
