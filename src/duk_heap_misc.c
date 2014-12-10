/*
 *  Support functions for duk_heap.
 */

#include "duk_internal.h"

#if defined(DUK_USE_DOUBLE_LINKED_HEAP) && defined(DUK_USE_REFERENCE_COUNTING)
/* arbitrary remove only works with double linked heap, and is only required by
 * reference counting so far.
 */
DUK_INTERNAL void duk_heap_remove_any_from_heap_allocated(duk_heap *heap, duk_heaphdr *hdr) {
	DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(hdr) != DUK_HTYPE_STRING);

	if (DUK_HEAPHDR_GET_PREV(heap, hdr)) {
		DUK_HEAPHDR_SET_NEXT(heap, DUK_HEAPHDR_GET_PREV(heap, hdr), DUK_HEAPHDR_GET_NEXT(heap, hdr));
	} else {
		heap->heap_allocated = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}
	if (DUK_HEAPHDR_GET_NEXT(heap, hdr)) {
		DUK_HEAPHDR_SET_PREV(heap, DUK_HEAPHDR_GET_NEXT(heap, hdr), DUK_HEAPHDR_GET_PREV(heap, hdr));
	} else {
		;
	}
}
#endif

DUK_INTERNAL void duk_heap_insert_into_heap_allocated(duk_heap *heap, duk_heaphdr *hdr) {
	DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(hdr) != DUK_HTYPE_STRING);

#ifdef DUK_USE_DOUBLE_LINKED_HEAP
	if (heap->heap_allocated) {
		DUK_ASSERT(DUK_HEAPHDR_GET_PREV(heap, heap->heap_allocated) == NULL);
		DUK_HEAPHDR_SET_PREV(heap, heap->heap_allocated, hdr);
	}
	DUK_HEAPHDR_SET_PREV(heap, hdr, NULL);
#endif
	DUK_HEAPHDR_SET_NEXT(heap, hdr, heap->heap_allocated);
	heap->heap_allocated = hdr;
}

#ifdef DUK_USE_INTERRUPT_COUNTER
DUK_INTERNAL void duk_heap_switch_thread(duk_heap *heap, duk_hthread *new_thr) {
	/* Copy currently active interrupt counter from the active thread
	 * back to the heap structure.  It doesn't need to be copied to
	 * the target thread, as the bytecode executor does that when it
	 * resumes execution for a new thread.
	 */
	if (heap->curr_thread != NULL) {
		heap->interrupt_counter = heap->curr_thread->interrupt_counter;
	}
	heap->curr_thread = new_thr;  /* may be NULL */
}
#endif  /* DUK_USE_INTERRUPT_COUNTER */
