/*
 *  duk_hbuffer allocation and freeing.
 */

#include "duk_internal.h"

DUK_INTERNAL duk_hbuffer *duk_hbuffer_alloc(duk_heap *heap, duk_size_t size, duk_bool_t dynamic) {
	duk_hbuffer *res = NULL;
	duk_size_t alloc_size;

	DUK_DDD(DUK_DDDPRINT("allocate hbuffer"));

	if (dynamic) {
		alloc_size = sizeof(duk_hbuffer_dynamic);
	} else {
		alloc_size = sizeof(duk_hbuffer_fixed) + size;
	}

#ifdef DUK_USE_ZERO_BUFFER_DATA
	/* zero everything */
	res = (duk_hbuffer *) DUK_ALLOC_ZEROED(heap, alloc_size);
#else
	res = (duk_hbuffer *) DUK_ALLOC(heap, alloc_size);
#endif
	if (!res) {
		goto error;
	}

#ifndef DUK_USE_ZERO_BUFFER_DATA
	/* if no buffer zeroing, zero the header anyway */
	DUK_MEMZERO((void *) res, dynamic ? sizeof(duk_hbuffer_dynamic) : sizeof(duk_hbuffer_fixed));
#endif

	if (dynamic) {
		duk_hbuffer_dynamic *h = (duk_hbuffer_dynamic *) res;
		void *ptr;
		if (size > 0) {
			DUK_DDD(DUK_DDDPRINT("dynamic buffer with nonzero size, alloc actual buffer"));
#ifdef DUK_USE_ZERO_BUFFER_DATA
			ptr = DUK_ALLOC_ZEROED(heap, size);
#else
			ptr = DUK_ALLOC(heap, size);
#endif
			if (!ptr) {
				/* Because size > 0, NULL check is correct */
				goto error;
			}

			h->curr_alloc = ptr;
			h->usable_size = size;  /* snug */
		} else {
#ifdef DUK_USE_EXPLICIT_NULL_INIT
			h->curr_alloc = NULL;
#endif
			DUK_ASSERT(h->usable_size == 0);
		}
	}

	res->size = size;

	DUK_HEAPHDR_SET_TYPE(&res->hdr, DUK_HTYPE_BUFFER);
	if (dynamic) {
		DUK_HBUFFER_SET_DYNAMIC(res);
	}
        DUK_HEAP_INSERT_INTO_HEAP_ALLOCATED(heap, &res->hdr);

	DUK_DDD(DUK_DDDPRINT("allocated hbuffer: %p", (void *) res));
	return res;

 error:
	DUK_DD(DUK_DDPRINT("hbuffer allocation failed"));

	DUK_FREE(heap, res);
	return NULL;
}

/* For indirect allocs. */

DUK_INTERNAL void *duk_hbuffer_get_dynalloc_ptr(void *ud) {
	duk_hbuffer_dynamic *buf = (duk_hbuffer_dynamic *) ud;
	return (void *) buf->curr_alloc;
}
