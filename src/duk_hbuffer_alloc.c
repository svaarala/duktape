/*
 *  duk_hbuffer allocation and freeing.
 */

#include "duk_internal.h"

duk_hbuffer *duk_hbuffer_alloc(duk_heap *heap, size_t size, int growable) {
	duk_hbuffer *res = NULL;
	size_t alloc_size;

	DUK_DDDPRINT("allocate hbuffer");

	if (growable) {
		alloc_size = sizeof(duk_hbuffer_growable);
	} else {
		/* FIXME: maybe remove safety NUL term for buffers? */
		alloc_size = sizeof(duk_hbuffer_fixed) + size + 1;  /* +1 for a safety nul term */
	}

	res = DUK_ALLOC(heap, alloc_size);
	if (!res) {
		goto error;
	}

	/* zero everything */
	memset(res, 0, alloc_size);

	if (growable) {
		duk_hbuffer_growable *h = (duk_hbuffer_growable *) res;
		void *ptr;
		if (size > 0) {
			/* FIXME: maybe remove safety NUL term for buffers? */
			DUK_DDDPRINT("growable buffer with nonzero size, alloc actual buffer");
			ptr = DUK_ALLOC(heap, size + 1);  /* +1 for a safety nul term */
			if (!ptr) {
				goto error;
			}
			memset(ptr, 0, size + 1);

			h->curr_alloc = ptr;
			h->usable_size = size;  /* snug */
		} else {
#ifdef DUK_USE_EXPLICIT_NULL_INIT
			h->curr_alloc = NULL;
#endif
		}
	}

	res->size = size;

	DUK_HEAPHDR_SET_TYPE(&res->hdr, DUK_HTYPE_BUFFER);
	if (growable) {
		DUK_HBUFFER_SET_GROWABLE(res);
	}
        DUK_HEAP_INSERT_INTO_HEAP_ALLOCATED(heap, &res->hdr);

	DUK_DDDPRINT("allocated hbuffer: %p", res);
	return res;

 error:
	DUK_DDPRINT("hbuffer allocation failed");

	DUK_FREE(heap, res);
	return NULL;
}

