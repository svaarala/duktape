/*
 *  duk_hbuffer allocation and freeing.
 */

#include "duk_internal.h"

duk_hbuffer *duk_hbuffer_alloc(duk_heap *heap, size_t size, int dynamic) {
	duk_hbuffer *res = NULL;
	size_t alloc_size;

	DUK_DDDPRINT("allocate hbuffer");

	if (dynamic) {
		alloc_size = sizeof(duk_hbuffer_dynamic);
	} else {
		/* FIXME: maybe remove safety NUL term for buffers? */
		alloc_size = sizeof(duk_hbuffer_fixed) + size + 1;  /* +1 for a safety nul term */
	}

	/* zero everything */
	res = (duk_hbuffer *) DUK_ALLOC_ZEROED(heap, alloc_size);
	if (!res) {
		goto error;
	}

	if (dynamic) {
		duk_hbuffer_dynamic *h = (duk_hbuffer_dynamic *) res;
		void *ptr;
		if (size > 0) {
			/* FIXME: maybe remove safety NUL term for buffers? */
			DUK_DDDPRINT("dynamic buffer with nonzero size, alloc actual buffer");
			ptr = DUK_ALLOC_ZEROED(heap, size + 1);  /* +1 for a safety nul term */
			if (!ptr) {
				goto error;
			}

			h->curr_alloc = ptr;
			h->usable_size = size;  /* snug */
		} else {
			/* FIXME: if safety NUL term IS included, having a NULL ptr
			 * for the buffer area is inconsistent.
			 */
#ifdef DUK_USE_EXPLICIT_NULL_INIT
			h->curr_alloc = NULL;
#endif
		}
	}

	res->size = size;

	DUK_HEAPHDR_SET_TYPE(&res->hdr, DUK_HTYPE_BUFFER);
	if (dynamic) {
		DUK_HBUFFER_SET_DYNAMIC(res);
	}
        DUK_HEAP_INSERT_INTO_HEAP_ALLOCATED(heap, &res->hdr);

	DUK_DDDPRINT("allocated hbuffer: %p", res);
	return res;

 error:
	DUK_DDPRINT("hbuffer allocation failed");

	DUK_FREE(heap, res);
	return NULL;
}

/* For indirect allocs. */

void *duk_hbuffer_get_dynalloc_ptr(void *ud) {
	duk_hbuffer_dynamic *buf = (duk_hbuffer_dynamic *) ud;
	return (void *) buf->curr_alloc;
}

