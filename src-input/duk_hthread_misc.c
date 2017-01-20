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

#if defined(DUK_USE_DEBUGGER_SUPPORT)
DUK_INTERNAL duk_uint_fast32_t duk_hthread_get_act_curr_pc(duk_hthread *thr, duk_activation *act) {
	duk_instr_t *bcode;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(act != NULL);
	DUK_UNREF(thr);

	/* XXX: store 'bcode' pointer to activation for faster lookup? */
	if (act->func && DUK_HOBJECT_IS_COMPFUNC(act->func)) {
		bcode = DUK_HCOMPFUNC_GET_CODE_BASE(thr->heap, (duk_hcompfunc *) (act->func));
		return (duk_uint_fast32_t) (act->curr_pc - bcode);
	}
	return 0;
}
#endif  /* DUK_USE_DEBUGGER_SUPPORT */

DUK_INTERNAL duk_uint_fast32_t duk_hthread_get_act_prev_pc(duk_hthread *thr, duk_activation *act) {
	duk_instr_t *bcode;
	duk_uint_fast32_t ret;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(act != NULL);
	DUK_UNREF(thr);

	if (act->func && DUK_HOBJECT_IS_COMPFUNC(act->func)) {
		bcode = DUK_HCOMPFUNC_GET_CODE_BASE(thr->heap, (duk_hcompfunc *) (act->func));
		ret = (duk_uint_fast32_t) (act->curr_pc - bcode);
		if (ret > 0) {
			ret--;
		}
		return ret;
	}
	return 0;
}

/* Write bytecode executor's curr_pc back to topmost activation (if any). */
DUK_INTERNAL void duk_hthread_sync_currpc(duk_hthread *thr) {
	duk_activation *act;

	DUK_ASSERT(thr != NULL);

	if (thr->ptr_curr_pc != NULL) {
		/* ptr_curr_pc != NULL only when bytecode executor is active. */
		DUK_ASSERT(thr->callstack_top > 0);
		DUK_ASSERT(thr->callstack_curr != NULL);
		act = thr->callstack_curr;
		DUK_ASSERT(act != NULL);
		act->curr_pc = *thr->ptr_curr_pc;
	}
}

DUK_INTERNAL void duk_hthread_sync_and_null_currpc(duk_hthread *thr) {
	duk_activation *act;

	DUK_ASSERT(thr != NULL);

	if (thr->ptr_curr_pc != NULL) {
		/* ptr_curr_pc != NULL only when bytecode executor is active. */
		DUK_ASSERT(thr->callstack_top > 0);
		DUK_ASSERT(thr->callstack_curr != NULL);
		act = thr->callstack_curr;
		DUK_ASSERT(act != NULL);
		act->curr_pc = *thr->ptr_curr_pc;
		thr->ptr_curr_pc = NULL;
	}
}
