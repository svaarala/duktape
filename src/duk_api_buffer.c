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

void duk_to_fixed_buffer(duk_context *ctx, int index) {
	duk_hbuffer_growable *h_src;
	char *data;
	size_t size;

	index = duk_require_normalize_index(ctx, index);

	h_src = (duk_hbuffer_growable *) duk_require_hbuffer(ctx, index);
	DUK_ASSERT(h_src != NULL);
	if (!DUK_HBUFFER_HAS_GROWABLE(h_src)) {
		return;
	}

	size = DUK_HBUFFER_GET_SIZE(h_src);
	data = duk_push_new_fixed_buffer(ctx, size);
	if (size > 0) {
		DUK_ASSERT(data != NULL);
		memcpy(data, DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(h_src), size);
	}

	duk_replace(ctx, index);
}

