/*
 *  duk_hbuffer allocation and freeing.
 */

#include "duk_internal.h"

DUK_INTERNAL duk_hbuffer *duk_hbuffer_alloc(duk_heap *heap, duk_size_t size, duk_bool_t dynamic) {
	duk_hbuffer *res = NULL;
	duk_size_t alloc_size;

	DUK_DDD(DUK_DDDPRINT("allocate hbuffer"));

	/* Size sanity check.  Should not be necessary because caller is
	 * required to check this, but we don't want to cause a segfault
	 * if the size wraps either in duk_size_t computation or when
	 * storing the size in a 16-bit field.
	 */
	if (size > DUK_HBUFFER_MAX_BYTELEN) {
		DUK_D(DUK_DPRINT("hbuffer alloc failed: size too large: %ld", (long) size));
		return NULL;
	}

	if (dynamic) {
		alloc_size = sizeof(duk_hbuffer_dynamic);
	} else {
		alloc_size = sizeof(duk_hbuffer_fixed) + size;
		DUK_ASSERT(alloc_size >= sizeof(duk_hbuffer_fixed));  /* no wrapping */
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

			DUK_HBUFFER_DYNAMIC_SET_DATA_PTR(heap, h, ptr);
			DUK_HBUFFER_DYNAMIC_SET_ALLOC_SIZE(h, size);  /* snug */
		} else {
#ifdef DUK_USE_EXPLICIT_NULL_INIT
			h->curr_alloc = NULL;
#endif
			DUK_ASSERT(DUK_HBUFFER_DYNAMIC_GET_ALLOC_SIZE(h) == 0);
		}
	}

	DUK_HBUFFER_SET_SIZE(res, size);

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

DUK_INTERNAL void *duk_hbuffer_get_dynalloc_ptr(duk_heap *heap, void *ud) {
	duk_hbuffer_dynamic *buf = (duk_hbuffer_dynamic *) ud;
	DUK_UNREF(heap);
	return (void *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(heap, buf);
}
