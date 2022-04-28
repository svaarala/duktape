#include "duk_internal.h"

#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
DUK_INTERNAL duk_uint_t duk_hbufobj_clamp_bytelength(duk_hbufobj *h_bufobj, duk_uint_t len) {
	duk_uint_t buf_size;
	duk_uint_t buf_avail;

	DUK_ASSERT(h_bufobj != NULL);
	DUK_ASSERT(h_bufobj->buf != NULL);

	buf_size = (duk_uint_t) DUK_HBUFFER_GET_SIZE(h_bufobj->buf);
	if (h_bufobj->offset > buf_size) {
		/* Slice starting point is beyond current length. */
		return 0;
	}
	buf_avail = buf_size - h_bufobj->offset;

	return buf_avail >= len ? len : buf_avail;
}

DUK_INTERNAL duk_uint8_t *duk_hbufobj_get_validated_data_ptr(duk_hthread *thr, duk_hbufobj *h, duk_uarridx_t idx) {
	duk_size_t byte_off;
	duk_small_uint_t elem_size;
	duk_uint8_t *data;

	DUK_ASSERT(h != NULL);

	/* Careful with wrapping: idx upshift may easily wrap, whereas
	 * length downshift won't.
	 */
	if (DUK_LIKELY(idx < DUK_HBUFOBJ_GET_LOGICAL_LENGTH(h))) {
		byte_off = idx << h->shift; /* no wrap assuming h_bufobj->length is valid */
		DUK_ASSERT(byte_off >= (duk_size_t) idx);
		elem_size = (duk_small_uint_t) (1U << h->shift);
		if (DUK_LIKELY(!DUK_HBUFOBJ_IS_DETACHED(h) && DUK_HBUFOBJ_VALID_BYTEOFFSET_EXCL(h, byte_off + elem_size))) {
			data = (duk_uint8_t *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h->buf) + h->offset + byte_off;
			return data;
		} else {
			/* Detached buffer or uncovered. */
			return NULL;
		}
	} else {
		/* Out-of-bounds. */
		return NULL;
	}
}

DUK_INTERNAL duk_uint8_t *duk_hbufobj_uint8array_get_validated_data_ptr(duk_hthread *thr, duk_hbufobj *h, duk_uarridx_t idx) {
	duk_size_t byte_off;
	duk_small_uint_t elem_size;
	duk_uint8_t *data;

	DUK_ASSERT(h != NULL);

	if (DUK_LIKELY(idx < DUK_HBUFOBJ_GET_LOGICAL_LENGTH(h))) {
		byte_off = idx;
		elem_size = 1;
		if (DUK_LIKELY(!DUK_HBUFOBJ_IS_DETACHED(h) && DUK_HBUFOBJ_VALID_BYTEOFFSET_EXCL(h, byte_off + elem_size))) {
			data = (duk_uint8_t *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h->buf) + h->offset + byte_off;
			return data;
		} else {
			/* Detached buffer or uncovered. */
			return NULL;
		}
	} else {
		/* Out-of-bounds. */
		return NULL;
	}
}

DUK_INTERNAL duk_bool_t duk_hbufobj_validate_and_write_top(duk_hthread *thr, duk_hbufobj *h, duk_uarridx_t idx) {
	duk_size_t byte_off;
	duk_small_uint_t elem_size;
	duk_uint8_t *data;

	DUK_ASSERT(h != NULL);

	/* Careful with wrapping: idx upshift may easily wrap, whereas
	 * length downshift won't.
	 */
	if (DUK_LIKELY(idx < DUK_HBUFOBJ_GET_LOGICAL_LENGTH(h))) {
		byte_off = idx << h->shift; /* no wrap assuming h_bufobj->length is valid */
		elem_size = (duk_small_uint_t) (1U << h->shift);
		if (DUK_LIKELY(!DUK_HBUFOBJ_IS_DETACHED(h) && DUK_HBUFOBJ_VALID_BYTEOFFSET_EXCL(h, byte_off + elem_size))) {
			data = (duk_uint8_t *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h->buf) + h->offset + byte_off;
			duk_hbufobj_validated_write(thr, h, data, elem_size);
			return 1;
		} else {
			/* Detached buffer or uncovered. */
			return 0;
		}
	} else {
		/* Out-of-bounds. */
		return 0;
	}
}

DUK_INTERNAL duk_bool_t duk_hbufobj_validate_and_read_push(duk_hthread *thr, duk_hbufobj *h, duk_uarridx_t idx) {
	duk_uint_t byte_off;
	duk_small_uint_t elem_size;
	duk_uint8_t *data;

	/* Careful with wrapping: idx upshift may easily wrap, whereas
	 * length downshift won't.
	 */
	if (DUK_LIKELY(idx < DUK_HBUFOBJ_GET_LOGICAL_LENGTH(h))) {
		byte_off = idx << h->shift; /* no wrap assuming h_bufobj->length is valid */
		elem_size = (duk_small_uint_t) (1U << h->shift);

		if (DUK_LIKELY(!DUK_HBUFOBJ_IS_DETACHED(h) && DUK_HBUFOBJ_VALID_BYTEOFFSET_EXCL(h, byte_off + elem_size))) {
			data = (duk_uint8_t *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h->buf) + h->offset + byte_off;
			duk_hbufobj_push_validated_read(thr, h, data, elem_size);
			return 1;
		} else {
			/* Detached or uncovered: treat like out-of-bounds. */
			return 0;
		}
	} else {
		return 0;
	}
}

#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */
