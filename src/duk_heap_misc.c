/*
 *  Support functions for duk_heap.
 */

#include "duk_internal.h"

#if defined(DUK_USE_DOUBLE_LINKED_HEAP) && defined(DUK_USE_REFERENCE_COUNTING)
/* arbitrary remove only works with double linked heap, and is only required by
 * reference counting so far.
 */
void duk_heap_remove_any_from_heap_allocated(duk_heap *heap, duk_heaphdr *hdr) {
	DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(hdr) != DUK_HTYPE_STRING);

	if (DUK_HEAPHDR_GET_PREV(hdr)) {
		DUK_HEAPHDR_SET_NEXT(DUK_HEAPHDR_GET_PREV(hdr), DUK_HEAPHDR_GET_NEXT(hdr));
	} else {
		heap->heap_allocated = DUK_HEAPHDR_GET_NEXT(hdr);
	}
	if (DUK_HEAPHDR_GET_NEXT(hdr)) {
		DUK_HEAPHDR_SET_PREV(DUK_HEAPHDR_GET_NEXT(hdr), DUK_HEAPHDR_GET_PREV(hdr));
	} else {
		;
	}
}
#endif

void duk_heap_insert_into_heap_allocated(duk_heap *heap, duk_heaphdr *hdr) {
	DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(hdr) != DUK_HTYPE_STRING);

#ifdef DUK_USE_DOUBLE_LINKED_HEAP
	if (heap->heap_allocated) {
		DUK_ASSERT(DUK_HEAPHDR_GET_PREV(heap->heap_allocated) == NULL);
		DUK_HEAPHDR_SET_PREV(heap->heap_allocated, hdr);
	}
	DUK_HEAPHDR_SET_PREV(hdr, NULL);
#endif
	DUK_HEAPHDR_SET_NEXT(hdr, heap->heap_allocated);
	heap->heap_allocated = hdr;
}

