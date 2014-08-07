/*
 *  Fast buffer writer with spare management.
 */

#include "duk_internal.h"

/*
 *  Macro support functions (use only macros in calling code)
 */

DUK_INTERNAL void duk_bw_init(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx, duk_hbuffer_dynamic *h_buf) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw_ctx != NULL);
	DUK_ASSERT(h_buf != NULL);
	DUK_UNREF(thr);

	bw_ctx->offset = 0;
	bw_ctx->length = DUK_HBUFFER_DYNAMIC_GET_SIZE(h_buf);
	bw_ctx->limit = (duk_uint8_t *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, h_buf) + bw_ctx->length;
	bw_ctx->buf = h_buf;
}

/* Get current write pointer.  After this you must call duk_bufwriter_ensure()
 * to start writing.
 */
DUK_INTERNAL duk_uint8_t *duk_bw_getptr(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw_ctx != NULL);
	DUK_UNREF(thr);

	return (duk_uint8_t *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, bw_ctx->buf) + bw_ctx->offset;
}

/* Resize target buffer for requested size.  Called by the macro only when the
 * fast path test (= there is space) fails.
 */
DUK_INTERNAL duk_uint8_t *duk_bw_resize(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx, duk_size_t sz, duk_uint8_t *ptr) {
	duk_size_t offset;
	duk_size_t add_sz;
	duk_size_t new_sz;
	duk_uint8_t *base;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw_ctx != NULL);
	DUK_ASSERT(ptr != NULL);

	/* 'offset' intentionally not updated to bw_ctx->offset until finish. */

	offset = (duk_size_t) (ptr - (duk_uint8_t *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, bw_ctx->buf));
	add_sz = (offset >> DUK_BW_SPARE_SHIFT) + DUK_BW_SPARE_ADD;
	new_sz = offset + sz + add_sz;
	if (new_sz < offset) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_BUFFER_TOO_LONG);
		return NULL;  /* not reachable */
	}
#if 0  /* for manual torture testing: tight allocation, useful with valgrind */
	new_sz = offset + sz;
#endif

	DUK_DD(DUK_DDPRINT("resize bufferwriter from %ld to %ld (add_sz=%ld)", (long) offset, (long) new_sz, (long) add_sz));

	/* XXX: simplify resize call when spare removed */
	duk_hbuffer_resize(thr, bw_ctx->buf, new_sz, new_sz);
	bw_ctx->length = new_sz;
	base = (duk_uint8_t *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, bw_ctx->buf);
	bw_ctx->limit = base + bw_ctx->length;
	return base + offset;
}

/* Finish writing for now, updates bw_ctx->offset. */
DUK_INTERNAL void duk_bw_finish(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx, duk_uint8_t *ptr) {
	duk_size_t offset;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw_ctx != NULL);
	DUK_ASSERT(ptr != NULL);
	DUK_UNREF(thr);

	offset = (duk_size_t) (ptr - (duk_uint8_t *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, bw_ctx->buf));
	bw_ctx->offset = offset;
}

/* Make buffer compact; caller must call duk_bw_finish() first to update bw_ctx->offset. */
DUK_INTERNAL void duk_bw_compact(duk_hthread *thr, duk_bufwriter_ctx *bw_ctx) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(bw_ctx != NULL);
	DUK_UNREF(thr);

	duk_hbuffer_resize(thr, bw_ctx->buf, bw_ctx->offset, bw_ctx->offset);
}

/*
 *  Macro support functions for reading/writing raw data.
 *
 *  These are done using mempcy to ensure they're valid even for unaligned
 *  reads/writes on platforms where alignment counts.  On x86 at least gcc
 *  is able to compile these into a bswap+mov.  "Always inline" is used to
 *  ensure these macros compile to minimal code.
 *
 *  Not really bufwriter related, but currently used together.
 */

DUK_ALWAYS_INLINE
DUK_INTERNAL duk_uint16_t duk_raw_read_u16_be(duk_uint8_t **p) {
	union {
		duk_uint8_t b[2];
		duk_uint16_t x;
	} u;

	DUK_MEMCPY((void *) u.b, (const void *) (*p), 2);
	u.x = DUK_NTOH16(u.x);
	*p += 2;
	return u.x;
}

DUK_ALWAYS_INLINE
DUK_INTERNAL duk_uint32_t duk_raw_read_u32_be(duk_uint8_t **p) {
	union {
		duk_uint8_t b[4];
		duk_uint32_t x;
	} u;

	DUK_MEMCPY((void *) u.b, (const void *) (*p), 4);
	u.x = DUK_NTOH32(u.x);
	*p += 4;
	return u.x;
}

DUK_ALWAYS_INLINE
DUK_INTERNAL duk_double_t duk_raw_read_double_be(duk_uint8_t **p) {
	duk_double_union du;
	union {
		duk_uint8_t b[4];
		duk_uint32_t x;
	} u;

	DUK_MEMCPY((void *) u.b, (const void *) (*p), 4);
	u.x = DUK_NTOH32(u.x);
	du.ui[DUK_DBL_IDX_UI0] = u.x;
	DUK_MEMCPY((void *) u.b, (const void *) (*p + 4), 4);
	u.x = DUK_NTOH32(u.x);
	du.ui[DUK_DBL_IDX_UI1] = u.x;
	*p += 8;

	return du.d;
}

DUK_ALWAYS_INLINE
DUK_INTERNAL void duk_raw_write_u16_be(duk_uint8_t **p, duk_uint16_t val) {
	union {
		duk_uint8_t b[2];
		duk_uint16_t x;
	} u;

	u.x = DUK_HTON16(val);
	DUK_MEMCPY((void *) (*p), (const void *) u.b, 2);
	*p += 2;
}

DUK_ALWAYS_INLINE
DUK_INTERNAL void duk_raw_write_u32_be(duk_uint8_t **p, duk_uint32_t val) {
	union {
		duk_uint8_t b[4];
		duk_uint32_t x;
	} u;

	u.x = DUK_HTON32(val);
	DUK_MEMCPY((void *) (*p), (const void *) u.b, 4);
	*p += 4;
}

DUK_ALWAYS_INLINE
DUK_INTERNAL void duk_raw_write_double_be(duk_uint8_t **p, duk_double_t val) {
	duk_double_union du;
	union {
		duk_uint8_t b[4];
		duk_uint32_t x;
	} u;

	du.d = val;
	u.x = du.ui[DUK_DBL_IDX_UI0];
	u.x = DUK_HTON32(u.x);
	DUK_MEMCPY((void *) (*p), (const void *) u.b, 4);
	u.x = du.ui[DUK_DBL_IDX_UI1];
	u.x = DUK_HTON32(u.x);
	DUK_MEMCPY((void *) (*p + 4), (const void *) u.b, 4);
	*p += 8;
}
