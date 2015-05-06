/*
 *  Buffer
 */

#include "duk_internal.h"

DUK_EXTERNAL void *duk_resize_buffer(duk_context *ctx, duk_idx_t index, duk_size_t new_size) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hbuffer_dynamic *h;

	DUK_ASSERT_CTX_VALID(ctx);

	h = (duk_hbuffer_dynamic *) duk_require_hbuffer(ctx, index);
	DUK_ASSERT(h != NULL);

	if (!DUK_HBUFFER_HAS_DYNAMIC(h)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_BUFFER_NOT_DYNAMIC);
	}

	/* maximum size check is handled by callee */
	duk_hbuffer_resize(thr, h, new_size, new_size);  /* snug */

	return DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, h);
}
