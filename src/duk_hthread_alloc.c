/*
 *  duk_hthread allocation and freeing.
 */

#include "duk_internal.h"

/*
 *  Allocate initial stacks for a thread.  Note that 'thr' must be reachable
 *  as a garbage collection may be triggered by the allocation attempts.
 *  Returns zero (without leaking memory) if init fails.
 */

int duk_hthread_init_stacks(duk_heap *heap, duk_hthread *thr) {
	size_t alloc_size;
	int i;

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
	thr->valstack = DUK_ALLOC(heap, alloc_size);
	if (!thr->valstack) {
		goto fail;
	}
	DUK_MEMSET(thr->valstack, 0, alloc_size);
	thr->valstack_end = thr->valstack + DUK_VALSTACK_INITIAL_SIZE;
	thr->valstack_bottom = thr->valstack;
	thr->valstack_top = thr->valstack;

	for (i = 0; i < DUK_VALSTACK_INITIAL_SIZE; i++) {
		DUK_TVAL_SET_UNDEFINED_UNUSED(&thr->valstack[i]);
	}

	/* callstack */
	alloc_size = sizeof(duk_activation) * DUK_CALLSTACK_INITIAL_SIZE;
	thr->callstack = DUK_ALLOC(heap, alloc_size);
	if (!thr->callstack) {
		goto fail;
	}
	DUK_MEMSET(thr->callstack, 0, alloc_size);
	thr->callstack_size = DUK_CALLSTACK_INITIAL_SIZE;
	DUK_ASSERT(thr->callstack_top == 0);

	/* catchstack */
	alloc_size = sizeof(duk_catcher) * DUK_CATCHSTACK_INITIAL_SIZE;
	thr->catchstack = DUK_ALLOC(heap, alloc_size);
	if (!thr->catchstack) {
		goto fail;
	}
	DUK_MEMSET(thr->catchstack, 0, alloc_size);
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

