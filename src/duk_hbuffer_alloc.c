/*
 *  duk_hbuffer allocation and freeing.
 */

#include "duk_internal.h"

DUK_INTERNAL duk_hbuffer *duk_hbuffer_alloc(duk_heap *heap, duk_size_t size, duk_small_uint_t flags) {
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

	if (flags & DUK_BUF_FLAG_DYNAMIC) {
		alloc_size = sizeof(duk_hbuffer_dynamic);
	} else {
		alloc_size = sizeof(duk_hbuffer_fixed) + size;
		DUK_ASSERT(alloc_size >= sizeof(duk_hbuffer_fixed));  /* no wrapping */
	}

	res = (duk_hbuffer *) DUK_ALLOC(heap, alloc_size);
	if (!res) {
		goto error;
	}

	/* zero everything unless requested not to do so */
#if defined(DUK_USE_ZERO_BUFFER_DATA)
	DUK_MEMZERO((void *) res,
	            (flags & DUK_BUF_FLAG_NOZERO) ?
	                ((flags & DUK_BUF_FLAG_DYNAMIC) ?
	                     sizeof(duk_hbuffer_dynamic) :
	                     sizeof(duk_hbuffer_fixed)) :
	                alloc_size);
#else
	DUK_MEMZERO((void *) res,
	            (flags & DUK_BUF_FLAG_DYNAMIC) ? sizeof(duk_hbuffer_dynamic) : sizeof(duk_hbuffer_fixed));
#endif

	if (flags & DUK_BUF_FLAG_DYNAMIC) {
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
	if (flags & DUK_BUF_FLAG_DYNAMIC) {
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
