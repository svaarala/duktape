/*
 *  Buffer
 */

#include "duk_internal.h"

void *duk_resize_buffer(duk_context *ctx, int index, size_t new_size) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hbuffer_growable *h;

	DUK_ASSERT(ctx != NULL);

	h = (duk_hbuffer_growable *) duk_require_hbuffer(ctx, index);
	DUK_ASSERT(h != NULL);

	if (!DUK_HBUFFER_HAS_GROWABLE(h)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "buffer is not growable");
	}

	duk_hbuffer_resize(thr, h, new_size, new_size);  /* snug */

	return DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(h);
}


