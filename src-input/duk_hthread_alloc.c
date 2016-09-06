/*
 *  duk_hthread allocation and freeing.
 */

#include "duk_internal.h"

/*
 *  Allocate initial stacks for a thread.  Note that 'thr' must be reachable
 *  as a garbage collection may be triggered by the allocation attempts.
 *  Returns zero (without leaking memory) if init fails.
 */

DUK_INTERNAL duk_bool_t duk_hthread_init_stacks(duk_heap *heap, duk_hthread *thr) {
	duk_size_t alloc_size;
	duk_size_t i;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->valstack == NULL);
	DUK_ASSERT(thr->valstack_end == NULL);
	DUK_ASSERT(thr->valstack_bottom == NULL);
	DUK_ASSERT(thr->valstack_top == NULL);
	DUK_ASSERT(thr->callstack == NULL);
	DUK_ASSERT(thr->catchstack == NULL);

	/* valstack */
	alloc_size = sizeof(duk_tval) * DUK_VALSTACK_INITIAL_SIZE;
	thr->valstack = (duk_tval *) DUK_ALLOC(heap, alloc_size);
	if (!thr->valstack) {
		goto fail;
	}
	DUK_MEMZERO(thr->valstack, alloc_size);
	thr->valstack_end = thr->valstack + DUK_VALSTACK_INITIAL_SIZE;
#if !defined(DUK_USE_PREFER_SIZE)
	thr->valstack_size = DUK_VALSTACK_INITIAL_SIZE;
#endif
	thr->valstack_bottom = thr->valstack;
	thr->valstack_top = thr->valstack;

	for (i = 0; i < DUK_VALSTACK_INITIAL_SIZE; i++) {
		DUK_TVAL_SET_UNDEFINED(&thr->valstack[i]);
	}

	/* callstack */
	alloc_size = sizeof(duk_activation) * DUK_CALLSTACK_INITIAL_SIZE;
	thr->callstack = (duk_activation *) DUK_ALLOC(heap, alloc_size);
	if (!thr->callstack) {
		goto fail;
	}
	DUK_MEMZERO(thr->callstack, alloc_size);
	thr->callstack_size = DUK_CALLSTACK_INITIAL_SIZE;
	DUK_ASSERT(thr->callstack_top == 0);

	/* catchstack */
	alloc_size = sizeof(duk_catcher) * DUK_CATCHSTACK_INITIAL_SIZE;
	thr->catchstack = (duk_catcher *) DUK_ALLOC(heap, alloc_size);
	if (!thr->catchstack) {
		goto fail;
	}
	DUK_MEMZERO(thr->catchstack, alloc_size);
	thr->catchstack_size = DUK_CATCHSTACK_INITIAL_SIZE;
	DUK_ASSERT(thr->catchstack_top == 0);

	return 1;

 fail:
	DUK_FREE(heap, thr->valstack);
	DUK_FREE(heap, thr->callstack);
	DUK_FREE(heap, thr->catchstack);

	thr->valstack = NULL;
	thr->callstack = NULL;
	thr->catchstack = NULL;
	return 0;
}

/* For indirect allocs. */

DUK_INTERNAL void *duk_hthread_get_valstack_ptr(duk_heap *heap, void *ud) {
	duk_hthread *thr = (duk_hthread *) ud;
	DUK_UNREF(heap);
	return (void *) thr->valstack;
}

DUK_INTERNAL void *duk_hthread_get_callstack_ptr(duk_heap *heap, void *ud) {
	duk_hthread *thr = (duk_hthread *) ud;
	DUK_UNREF(heap);
	return (void *) thr->callstack;
}

DUK_INTERNAL void *duk_hthread_get_catchstack_ptr(duk_heap *heap, void *ud) {
	duk_hthread *thr = (duk_hthread *) ud;
	DUK_UNREF(heap);
	return (void *) thr->catchstack;
}
