/*
 *  Duktape debugger
 */

#include "duk_internal.h"

#if defined(DUK_USE_DEBUGGER_SUPPORT)

/*
 *  Helper structs
 */

typedef union {
	void *p;
	duk_uint_t b[1];
	/* Use b[] to access the size of the union, which is strictly not
	 * correct.  Can't use fixed size unless there's feature detection
	 * for pointer byte size.
	 */
} duk__ptr_union;

/*
 *  Detach handling
 */

#define DUK__SET_CONN_BROKEN(thr) do { \
		/* For now shared handler is fine. */ \
		duk_debug_do_detach((thr)->heap); \
	} while (0)

DUK_INTERNAL void duk_debug_do_detach(duk_heap *heap) {
	/* Can be called muliple times with no harm. */

	heap->dbg_read_cb = NULL;
	heap->dbg_write_cb = NULL;
	heap->dbg_peek_cb = NULL;
	heap->dbg_read_flush_cb = NULL;
	heap->dbg_write_flush_cb = NULL;
	if (heap->dbg_detached_cb) {
		heap->dbg_detached_cb(heap->dbg_udata);
	}
	heap->dbg_detached_cb = NULL;
	heap->dbg_udata = NULL;
	heap->dbg_processing = 0;
	heap->dbg_paused = 0;
	heap->dbg_state_dirty = 0;
	heap->dbg_step_type = 0;
	heap->dbg_step_thread = NULL;
	heap->dbg_step_csindex = 0;
	heap->dbg_step_startline = 0;

	/* Ensure there are no stale active breakpoint pointers.
	 * Breakpoint list is currently kept - we could empty it
	 * here but we'd need to handle refcounts correctly, and
	 * we'd need a 'thr' reference for that.
	 *
	 * XXX: clear breakpoint on either attach or detach?
	 */
	heap->dbg_breakpoints_active[0] = (duk_breakpoint *) NULL;
}

/*
 *  Debug connection peek and flush primitives
 */

DUK_INTERNAL duk_bool_t duk_debug_read_peek(duk_hthread *thr) {
	duk_heap *heap;

	DUK_ASSERT(thr != NULL);
	heap = thr->heap;

	if (heap->dbg_read_cb == NULL) {
		DUK_D(DUK_DPRINT("attempt to peek in detached state, return zero (= no data)"));
		return 0;
	}
	if (heap->dbg_peek_cb == NULL) {
		DUK_DD(DUK_DDPRINT("no peek callback, return zero (= no data)"));
		return 0;
	}

	return (duk_bool_t) (heap->dbg_peek_cb(heap->dbg_udata) > 0);
}

DUK_INTERNAL void duk_debug_read_flush(duk_hthread *thr) {
	duk_heap *heap;

	DUK_ASSERT(thr != NULL);
	heap = thr->heap;

	if (heap->dbg_read_cb == NULL) {
		DUK_D(DUK_DPRINT("attempt to read flush in detached state, ignore"));
		return;
	}
	if (heap->dbg_read_flush_cb == NULL) {
		DUK_DD(DUK_DDPRINT("no read flush callback, ignore"));
		return;
	}

	heap->dbg_read_flush_cb(heap->dbg_udata);
}

DUK_INTERNAL void duk_debug_write_flush(duk_hthread *thr) {
	duk_heap *heap;

	DUK_ASSERT(thr != NULL);
	heap = thr->heap;

	if (heap->dbg_read_cb == NULL) {
		DUK_D(DUK_DPRINT("attempt to write flush in detached state, ignore"));
		return;
	}
	if (heap->dbg_write_flush_cb == NULL) {
		DUK_DD(DUK_DDPRINT("no write flush callback, ignore"));
		return;
	}

	heap->dbg_write_flush_cb(heap->dbg_udata);
}

/*
 *  Debug connection skip primitives
 */

/* Skip fully. */
DUK_INTERNAL void duk_debug_skip_bytes(duk_hthread *thr, duk_size_t length) {
	duk_uint8_t dummy[64];
	duk_size_t now;

	DUK_ASSERT(thr != NULL);

	while (length > 0) {
		now = (length > sizeof(dummy) ? sizeof(dummy) : length);
		duk_debug_read_bytes(thr, dummy, now);
		length -= now;
	}
}

DUK_INTERNAL void duk_debug_skip_byte(duk_hthread *thr) {
	DUK_ASSERT(thr != NULL);

	(void) duk_debug_read_byte(thr);
}

/*
 *  Debug connection read primitives
 */

/* Read fully. */
DUK_INTERNAL void duk_debug_read_bytes(duk_hthread *thr, duk_uint8_t *data, duk_size_t length) {
	duk_heap *heap;
	duk_uint8_t *p;
	duk_size_t left;
	duk_size_t got;

	DUK_ASSERT(thr != NULL);
	heap = thr->heap;

	if (heap->dbg_read_cb == NULL) {
		DUK_D(DUK_DPRINT("attempt to read %ld bytes in detached state, return zero data", (long) length));
		goto fail;
	}

	p = data;
	for (;;) {
		left = (duk_size_t) ((data + length) - p);
		if (left == 0) {
			break;
		}
		DUK_ASSERT(heap->dbg_read_cb != NULL);
		DUK_ASSERT(left >= 1);
#if defined(DUK_USE_DEBUGGER_TRANSPORT_TORTURE)
		left = 1;
#endif
		got = heap->dbg_read_cb(heap->dbg_udata, (char *) p, left);
		if (got == 0 || got > left) {
			DUK_D(DUK_DPRINT("connection error during read, return zero data"));
			DUK__SET_CONN_BROKEN(thr);
			goto fail;
		}
		p += got;
	}
	return;

 fail:
	DUK_MEMZERO((void *) data, (size_t) length);
}

DUK_INTERNAL duk_uint8_t duk_debug_read_byte(duk_hthread *thr) {
	duk_heap *heap;
	duk_size_t got;
	duk_uint8_t x;

	DUK_ASSERT(thr != NULL);
	heap = thr->heap;

	if (heap->dbg_read_cb == NULL) {
		DUK_D(DUK_DPRINT("attempt to read 1 bytes in detached state, return zero data"));
		return 0;
	}

	x = 0;  /* just in case callback is broken and won't write 'x' */
	DUK_ASSERT(heap->dbg_read_cb != NULL);
	got = heap->dbg_read_cb(heap->dbg_udata, (char *) (&x), 1);
	if (got != 1) {
		DUK_D(DUK_DPRINT("connection error during read, return zero data"));
		DUK__SET_CONN_BROKEN(thr);
		return 0;
	}

	return x;
}

DUK_LOCAL duk_uint32_t duk__debug_read_uint32_raw(duk_hthread *thr) {
	duk_uint8_t buf[4];

	DUK_ASSERT(thr != NULL);

	duk_debug_read_bytes(thr, buf, 4);
	return ((duk_uint32_t) buf[0] << 24) |
	       ((duk_uint32_t) buf[1] << 16) |
	       ((duk_uint32_t) buf[2] << 8) |
	       (duk_uint32_t) buf[3];
}

DUK_LOCAL duk_uint32_t duk__debug_read_int32_raw(duk_hthread *thr) {
	return (duk_int32_t) duk__debug_read_uint32_raw(thr);
}

DUK_LOCAL duk_uint16_t duk__debug_read_uint16_raw(duk_hthread *thr) {
	duk_uint8_t buf[2];

	DUK_ASSERT(thr != NULL);

	duk_debug_read_bytes(thr, buf, 2);
	return ((duk_uint16_t) buf[0] << 8) |
	       (duk_uint16_t) buf[1];
}

DUK_INTERNAL duk_int32_t duk_debug_read_int(duk_hthread *thr) {
	duk_small_uint_t x;
	duk_small_uint_t t;

	DUK_ASSERT(thr != NULL);

	x = duk_debug_read_byte(thr);
	if (x >= 0xc0) {
		t = duk_debug_read_byte(thr);
		return (duk_int32_t) (((x - 0xc0) << 8) + t);
	} else if (x >= 0x80) {
		return (duk_int32_t) (x - 0x80);
	} else if (x == 0x10) {
		return (duk_int32_t) duk__debug_read_uint32_raw(thr);
	}

	DUK_D(DUK_DPRINT("debug connection error: failed to decode int"));
	DUK__SET_CONN_BROKEN(thr);
	return 0;
}

DUK_LOCAL duk_hstring *duk__debug_read_hstring_raw(duk_hthread *thr, duk_uint32_t len) {
	duk_context *ctx = (duk_context *) thr;
	duk_uint8_t buf[31];
	duk_uint8_t *p;

	if (len <= sizeof(buf)) {
		duk_debug_read_bytes(thr, buf, (duk_size_t) len);
		duk_push_lstring(ctx, (const char *) buf, (duk_size_t) len);
	} else {
		p = (duk_uint8_t *) duk_push_fixed_buffer(ctx, (duk_size_t) len);
		DUK_ASSERT(p != NULL);
		duk_debug_read_bytes(thr, p, (duk_size_t) len);
		duk_to_string(ctx, -1);
	}

	return duk_require_hstring(ctx, -1);
}

DUK_INTERNAL duk_hstring *duk_debug_read_hstring(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_small_uint_t x;
	duk_uint32_t len;

	DUK_ASSERT(thr != NULL);

	x = duk_debug_read_byte(thr);
	if (x >= 0x60 && x <= 0x7f) {
		/* For short strings, use a fixed temp buffer. */
		len = (duk_uint32_t) (x - 0x60);
	} else if (x == 0x12) {
		len = (duk_uint32_t) duk__debug_read_uint16_raw(thr);
	} else if (x == 0x11) {
		len = (duk_uint32_t) duk__debug_read_uint32_raw(thr);
	} else {
		goto fail;
	}

	return duk__debug_read_hstring_raw(thr, len);

 fail:
	DUK_D(DUK_DPRINT("debug connection error: failed to decode int"));
	DUK__SET_CONN_BROKEN(thr);
	duk_push_hstring_stridx(thr, DUK_STRIDX_EMPTY_STRING);  /* always push some string */
	return duk_require_hstring(ctx, -1);
}

DUK_LOCAL duk_hbuffer *duk__debug_read_hbuffer_raw(duk_hthread *thr, duk_uint32_t len) {
	duk_context *ctx = (duk_context *) thr;
	duk_uint8_t *p;

	p = (duk_uint8_t *) duk_push_fixed_buffer(ctx, (duk_size_t) len);
	DUK_ASSERT(p != NULL);
	duk_debug_read_bytes(thr, p, (duk_size_t) len);

	return duk_require_hbuffer(ctx, -1);
}

DUK_LOCAL const void *duk__debug_read_pointer_raw(duk_hthread *thr) {
	duk_small_uint_t x;
	volatile duk__ptr_union pu;

	DUK_ASSERT(thr != NULL);

	x = duk_debug_read_byte(thr);
	if (x != sizeof(pu)) {
		goto fail;
	}
	duk_debug_read_bytes(thr, (duk_uint8_t *) &pu.p, sizeof(pu));
#if defined(DUK_USE_INTEGER_LE)
	duk_byteswap_bytes((duk_uint8_t *) pu.b, sizeof(pu));
#endif
	return (const void *) pu.p;

 fail:
	DUK_D(DUK_DPRINT("debug connection error: failed to decode pointer"));
	DUK__SET_CONN_BROKEN(thr);
	return (const void *) NULL;
}

DUK_LOCAL duk_double_t duk__debug_read_double_raw(duk_hthread *thr) {
	duk_double_union du;

	DUK_ASSERT(sizeof(du.uc) == 8);
	duk_debug_read_bytes(thr, (duk_uint8_t *) du.uc, sizeof(du.uc));
	DUK_DBLUNION_BSWAP(&du);
	return du.d;
}

DUK_INTERNAL void duk_debug_read_tval(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_uint8_t x;
	duk_uint_t t;
	duk_uint32_t len;

	DUK_ASSERT(thr != NULL);

	x = duk_debug_read_byte(thr);

	if (x >= 0xc0) {
		t = (duk_uint_t) (x - 0xc0);
		t = (t << 8) + duk_debug_read_byte(thr);
		duk_push_uint(ctx, (duk_uint_t) t);
		return;
	}
	if (x >= 0x80) {
		duk_push_uint(ctx, (duk_uint_t) (x - 0x80));
		return;
	}
	if (x >= 0x60) {
		len = (duk_uint32_t) (x - 0x60);
		duk__debug_read_hstring_raw(thr, len);
		return;
	}

	switch (x) {
	case 0x10: {
		duk_int32_t i = duk__debug_read_int32_raw(thr);
		duk_push_i32(ctx, i);
		break;
	}
	case 0x11:
		len = duk__debug_read_uint32_raw(thr);
		duk__debug_read_hstring_raw(thr, len);
		break;
	case 0x12:
		len = duk__debug_read_uint16_raw(thr);
		duk__debug_read_hstring_raw(thr, len);
		break;
	case 0x13:
		len = duk__debug_read_uint32_raw(thr);
		duk__debug_read_hbuffer_raw(thr, len);
		break;
	case 0x14:
		len = duk__debug_read_uint16_raw(thr);
		duk__debug_read_hbuffer_raw(thr, len);
		break;
	case 0x15:
		duk_push_unused(ctx);
		break;
	case 0x16:
		duk_push_undefined(ctx);
		break;
	case 0x17:
		duk_push_null(ctx);
		break;
	case 0x18:
		duk_push_true(ctx);
		break;
	case 0x19:
		duk_push_false(ctx);
		break;
	case 0x1a: {
		duk_double_t d;
		d = duk__debug_read_double_raw(thr);
		duk_push_number(ctx, d);
		break;
	}
	case 0x1b:
		/* XXX: not needed for now, so not implemented */
		DUK_D(DUK_DPRINT("reading object values unimplemented"));
		goto fail;
	case 0x1c: {
		const void *ptr;
		ptr = duk__debug_read_pointer_raw(thr);
		duk_push_pointer(thr, (void *) ptr);
		break;
	}
	case 0x1d:
		/* XXX: not needed for now, so not implemented */
		DUK_D(DUK_DPRINT("reading lightfunc values unimplemented"));
		goto fail;
	case 0x1e: {
		duk_heaphdr *h;
		h = (duk_heaphdr *) duk__debug_read_pointer_raw(thr);
		duk_push_heapptr(thr, (void *) h);
		break;
	}
	default:
		goto fail;
	}

	return;

 fail:
	DUK_D(DUK_DPRINT("debug connection error: failed to decode tval"));
	DUK__SET_CONN_BROKEN(thr);
}

/*
 *  Debug connection write primitives
 */

/* Write fully. */
DUK_INTERNAL void duk_debug_write_bytes(duk_hthread *thr, const duk_uint8_t *data, duk_size_t length) {
	duk_heap *heap;
	const duk_uint8_t *p;
	duk_size_t left;
	duk_size_t got;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(length == 0 || data != NULL);
	heap = thr->heap;

	if (heap->dbg_write_cb == NULL) {
		DUK_D(DUK_DPRINT("attempt to write %ld bytes in detached state, ignore", (long) length));
		return;
	}
	if (length == 0) {
		/* Avoid doing an actual write callback with length == 0,
		 * because that's reserved for a write flush.
		 */
		return;
	}
	DUK_ASSERT(data != NULL);

	p = data;
	for (;;) {
		left = (duk_size_t) ((data + length) - p);
		if (left == 0) {
			break;
		}
		DUK_ASSERT(heap->dbg_write_cb != NULL);
		DUK_ASSERT(left >= 1);
#if defined(DUK_USE_DEBUGGER_TRANSPORT_TORTURE)
		left = 1;
#endif
		got = heap->dbg_write_cb(heap->dbg_udata, (const char *) p, left);
		if (got == 0 || got > left) {
			DUK_D(DUK_DPRINT("connection error during write"));
			DUK__SET_CONN_BROKEN(thr);
			return;
		}
		p += got;
	}
}

DUK_INTERNAL void duk_debug_write_byte(duk_hthread *thr, duk_uint8_t x) {
	duk_heap *heap;
	duk_size_t got;

	DUK_ASSERT(thr != NULL);
	heap = thr->heap;

	if (heap->dbg_write_cb == NULL) {
		DUK_D(DUK_DPRINT("attempt to write 1 bytes in detached state, ignore"));
		return;
	}

	DUK_ASSERT(heap->dbg_write_cb != NULL);
	got = heap->dbg_write_cb(heap->dbg_udata, (const char *) (&x), 1);
	if (got != 1) {
		DUK_D(DUK_DPRINT("connection error during write"));
		DUK__SET_CONN_BROKEN(thr);
	}
}

DUK_INTERNAL void duk_debug_write_unused(duk_hthread *thr) {
	duk_debug_write_byte(thr, 0x15);
}

DUK_INTERNAL void duk_debug_write_undefined(duk_hthread *thr) {
	duk_debug_write_byte(thr, 0x16);
}

/* Write signed 32-bit integer. */
DUK_INTERNAL void duk_debug_write_int(duk_hthread *thr, duk_int32_t x) {
	duk_uint8_t buf[5];
	duk_size_t len;

	DUK_ASSERT(thr != NULL);

	if (x >= 0 && x <= 0x3fL) {
		buf[0] = (duk_uint8_t) (0x80 + x);
		len = 1;
	} else if (x >= 0 && x <= 0x3fffL) {
		buf[0] = (duk_uint8_t) (0xc0 + (x >> 8));
		buf[1] = (duk_uint8_t) (x & 0xff);
		len = 2;
	} else {
		/* Signed integers always map to 4 bytes now. */
		buf[0] = (duk_uint8_t) 0x10;
		buf[1] = (duk_uint8_t) ((x >> 24) & 0xff);
		buf[2] = (duk_uint8_t) ((x >> 16) & 0xff);
		buf[3] = (duk_uint8_t) ((x >> 8) & 0xff);
		buf[4] = (duk_uint8_t) (x & 0xff);
		len = 5;
	}
	duk_debug_write_bytes(thr, buf, len);
}

/* Write unsigned 32-bit integer. */
DUK_INTERNAL void duk_debug_write_uint(duk_hthread *thr, duk_uint32_t x) {
	/* XXX: there's currently no need to support full 32-bit unsigned
	 * integer range in practice.  If that becomes necessary, add a new
	 * dvalue type or encode as an IEEE double.
	 */
	duk_debug_write_int(thr, (duk_int32_t) x);
}

DUK_INTERNAL void duk_debug_write_strbuf(duk_hthread *thr, const char *data, duk_size_t length, duk_uint8_t marker_base) {
	duk_uint8_t buf[5];
	duk_size_t buflen;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(length == 0 || data != NULL);

	if (length <= 0x1fUL && marker_base == 0x11) {
		/* For strings, special form for short lengths. */
		buf[0] = (duk_uint8_t) (0x60 + length);
		buflen = 1;
	} else if (length <= 0xffffUL) {
		buf[0] = (duk_uint8_t) (marker_base + 1);
		buf[1] = (duk_uint8_t) (length >> 8);
		buf[2] = (duk_uint8_t) (length & 0xff);
		buflen = 3;
	} else {
		buf[0] = (duk_uint8_t) marker_base;
		buf[1] = (duk_uint8_t) (length >> 24);
		buf[2] = (duk_uint8_t) ((length >> 16) & 0xff);
		buf[3] = (duk_uint8_t) ((length >> 8) & 0xff);
		buf[4] = (duk_uint8_t) (length & 0xff);
		buflen = 5;
	}

	duk_debug_write_bytes(thr, (const duk_uint8_t *) buf, buflen);
	duk_debug_write_bytes(thr, (const duk_uint8_t *) data, length);
}

DUK_INTERNAL void duk_debug_write_string(duk_hthread *thr, const char *data, duk_size_t length) {
	duk_debug_write_strbuf(thr, data, length, 0x11);
}

DUK_INTERNAL void duk_debug_write_cstring(duk_hthread *thr, const char *data) {
	DUK_ASSERT(thr != NULL);

	duk_debug_write_string(thr,
	                       data,
	                       data ? DUK_STRLEN(data) : 0);
}

DUK_INTERNAL void duk_debug_write_hstring(duk_hthread *thr, duk_hstring *h) {
	DUK_ASSERT(thr != NULL);

	/* XXX: differentiate null pointer from empty string? */
	duk_debug_write_string(thr,
	                       (h != NULL ? (const char *) DUK_HSTRING_GET_DATA(h) : NULL),
	                       (h != NULL ? (duk_size_t) DUK_HSTRING_GET_BYTELEN(h) : 0));
}

DUK_INTERNAL void duk_debug_write_buffer(duk_hthread *thr, const char *data, duk_size_t length) {
	duk_debug_write_strbuf(thr, data, length, 0x13);
}

DUK_INTERNAL void duk_debug_write_hbuffer(duk_hthread *thr, duk_hbuffer *h) {
	DUK_ASSERT(thr != NULL);

	duk_debug_write_buffer(thr,
	                       (h != NULL ? (const char *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h) : NULL),
	                       (h != NULL ? (duk_size_t) DUK_HBUFFER_GET_SIZE(h) : 0));
}

DUK_LOCAL void duk__debug_write_pointer_raw(duk_hthread *thr, const void *ptr, duk_uint8_t ibyte) {
	duk_uint8_t buf[2];
	volatile duk__ptr_union pu;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(sizeof(ptr) >= 1 && sizeof(ptr) <= 16);
	/* ptr may be NULL */

	buf[0] = ibyte;
	buf[1] = sizeof(pu);
	duk_debug_write_bytes(thr, buf, 2);
	pu.p = (void *) ptr;
#if defined(DUK_USE_INTEGER_LE)
	duk_byteswap_bytes((duk_uint8_t *) pu.b, sizeof(pu));
#endif
	duk_debug_write_bytes(thr, (const duk_uint8_t *) &pu.p, (duk_size_t) sizeof(pu));
}

DUK_INTERNAL void duk_debug_write_pointer(duk_hthread *thr, const void *ptr) {
	duk__debug_write_pointer_raw(thr, ptr, 0x1c);
}

#if defined(DUK_USE_DEBUGGER_DUMPHEAP)
DUK_INTERNAL void duk_debug_write_heapptr(duk_hthread *thr, duk_heaphdr *h) {
	duk__debug_write_pointer_raw(thr, (const void *) h, 0x1e);
}
#endif  /* DUK_USE_DEBUGGER_DUMPHEAP */

DUK_INTERNAL void duk_debug_write_hobject(duk_hthread *thr, duk_hobject *obj) {
	duk_uint8_t buf[3];
	volatile duk__ptr_union pu;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(sizeof(obj) >= 1 && sizeof(obj) <= 16);
	DUK_ASSERT(obj != NULL);

	buf[0] = 0x1b;
	buf[1] = (duk_uint8_t) DUK_HOBJECT_GET_CLASS_NUMBER(obj);
	buf[2] = sizeof(pu);
	duk_debug_write_bytes(thr, buf, 3);
	pu.p = (void *) obj;
#if defined(DUK_USE_INTEGER_LE)
	duk_byteswap_bytes((duk_uint8_t *) pu.b, sizeof(pu));
#endif
	duk_debug_write_bytes(thr, (const duk_uint8_t *) &pu.p, (duk_size_t) sizeof(pu));
}

DUK_INTERNAL void duk_debug_write_tval(duk_hthread *thr, duk_tval *tv) {
	duk_c_function lf_func;
	duk_small_uint_t lf_flags;
	duk_uint8_t buf[4];
	duk_double_union du;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED:
		duk_debug_write_byte(thr,
		                     DUK_TVAL_IS_UNDEFINED_UNUSED(tv) ? 0x15 : 0x16);
		break;
	case DUK_TAG_NULL:
		duk_debug_write_byte(thr, 0x17);
		break;
	case DUK_TAG_BOOLEAN:
		DUK_ASSERT(DUK_TVAL_GET_BOOLEAN(tv) == 0 ||
		           DUK_TVAL_GET_BOOLEAN(tv) == 1);
		duk_debug_write_byte(thr, DUK_TVAL_GET_BOOLEAN(tv) ? 0x18 : 0x19);
		break;
	case DUK_TAG_POINTER:
		duk_debug_write_pointer(thr, (const void *) DUK_TVAL_GET_POINTER(tv));
		break;
	case DUK_TAG_LIGHTFUNC:
		DUK_TVAL_GET_LIGHTFUNC(tv, lf_func, lf_flags);
		buf[0] = 0x1d;
		buf[1] = (duk_uint8_t) (lf_flags >> 8);
		buf[2] = (duk_uint8_t) (lf_flags & 0xff);
		buf[3] = sizeof(lf_func);
		duk_debug_write_bytes(thr, buf, 4);
		duk_debug_write_bytes(thr, (const duk_uint8_t *) &lf_func, sizeof(lf_func));
		break;
	case DUK_TAG_STRING:
		duk_debug_write_hstring(thr, DUK_TVAL_GET_STRING(tv));
		break;
	case DUK_TAG_OBJECT:
		duk_debug_write_hobject(thr, DUK_TVAL_GET_OBJECT(tv));
		break;
	case DUK_TAG_BUFFER:
		duk_debug_write_hbuffer(thr, DUK_TVAL_GET_BUFFER(tv));
		break;
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT:
#endif
	default:
		/* Numbers are normalized to big (network) endian. */
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		du.d = DUK_TVAL_GET_NUMBER(tv);
		DUK_DBLUNION_BSWAP(&du);

		duk_debug_write_byte(thr, 0x1a);
		duk_debug_write_bytes(thr, (const duk_uint8_t *) du.uc, sizeof(du.uc));
	}
}

#if defined(DUK_USE_DEBUGGER_DUMPHEAP)
/* Variant for writing duk_tvals so that any heap allocated values are
 * written out as tagged heap pointers.
 */
DUK_LOCAL void duk__debug_write_tval_heapptr(duk_hthread *thr, duk_tval *tv) {
	if (DUK_TVAL_IS_HEAP_ALLOCATED(tv)) {
		duk_heaphdr *h = DUK_TVAL_GET_HEAPHDR(tv);
		duk_debug_write_heapptr(thr, h);
	} else {
		duk_debug_write_tval(thr, tv);
	}
}
#endif  /* DUK_USE_DEBUGGER_DUMPHEAP */

/*
 *  Debug connection message write helpers
 */

#if 0  /* unused */
DUK_INTERNAL void duk_debug_write_request(duk_hthread *thr, duk_small_uint_t command) {
	duk_debug_write_byte(thr, DUK_DBG_MARKER_REQUEST);
	duk_debug_write_int(thr, command);
}
#endif

DUK_INTERNAL void duk_debug_write_reply(duk_hthread *thr) {
	duk_debug_write_byte(thr, DUK_DBG_MARKER_REPLY);
}

DUK_INTERNAL void duk_debug_write_error_eom(duk_hthread *thr, duk_small_uint_t err_code, const char *msg) {
	/* Allow NULL 'msg' */
	duk_debug_write_byte(thr, DUK_DBG_MARKER_ERROR);
	duk_debug_write_int(thr, (duk_int32_t) err_code);
	duk_debug_write_cstring(thr, msg);
	duk_debug_write_eom(thr);
}

DUK_INTERNAL void duk_debug_write_notify(duk_hthread *thr, duk_small_uint_t command) {
	duk_debug_write_byte(thr, DUK_DBG_MARKER_NOTIFY);
	duk_debug_write_int(thr, command);
}

DUK_INTERNAL void duk_debug_write_eom(duk_hthread *thr) {
	duk_debug_write_byte(thr, DUK_DBG_MARKER_EOM);

	/* As an initial implementation, write flush after every EOM (and the
	 * version identifier).  A better implementation would flush only when
	 * Duktape is finished processing messages so that a flush only happens
	 * after all outbound messages are finished on that occasion.
	 */
	duk_debug_write_flush(thr);
}

/*
 *  Status message and helpers
 */

DUK_INTERNAL duk_uint_fast32_t duk_debug_curr_line(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_activation *act;
	duk_uint_fast32_t line;
	duk_uint_fast32_t pc;

	if (thr->callstack_top == 0) {
		return 0;
	}
	act = thr->callstack + thr->callstack_top - 1;

	/* act->pc indicates the next instruction about to be executed.  This
	 * is usually correct, but for the 'debugger' statement it will be the
	 * instruction after that.
	 */

	pc = (duk_uint_fast32_t) act->pc;

	/* XXX: this should be optimized to be a raw query and avoid valstack
	 * operations if possible.
	 */
	duk_push_hobject(ctx, act->func);
	line = duk_hobject_pc2line_query(ctx, -1, pc);
	duk_pop(ctx);
	return line;
}

DUK_INTERNAL void duk_debug_send_status(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_activation *act;

	duk_debug_write_notify(thr, DUK_DBG_CMD_STATUS);
	duk_debug_write_int(thr, thr->heap->dbg_paused);

	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);  /* unsigned */
	if (thr->callstack_top == 0) {
		duk_debug_write_undefined(thr);
		duk_debug_write_undefined(thr);
		duk_debug_write_int(thr, 0);
		duk_debug_write_int(thr, 0);
	} else {
		act = thr->callstack + thr->callstack_top - 1;
		duk_push_hobject(ctx, act->func);
		duk_get_prop_string(ctx, -1, "fileName");
		duk_safe_to_string(ctx, -1);
		duk_debug_write_hstring(thr, duk_require_hstring(ctx, -1));
		duk_get_prop_string(ctx, -2, "name");
		duk_safe_to_string(ctx, -1);
		duk_debug_write_hstring(thr, duk_require_hstring(ctx, -1));
		duk_pop_3(ctx);
		duk_debug_write_uint(thr, (duk_uint32_t) duk_debug_curr_line(thr));
		duk_debug_write_uint(thr, (duk_uint32_t) act->pc);
	}

	duk_debug_write_eom(thr);
}

/*
 *  Debug message processing
 */

/* Skip dvalue. */
DUK_LOCAL duk_bool_t duk__debug_skip_dvalue(duk_hthread *thr) {
	duk_uint8_t x;
	duk_uint32_t len;

	x = duk_debug_read_byte(thr);

	if (x >= 0xc0) {
		duk_debug_skip_byte(thr);
		return 0;
	}
	if (x >= 0x80) {
		return 0;
	}
	if (x >= 0x60) {
		duk_debug_skip_bytes(thr, x - 0x60);
		return 0;
	}
	switch(x) {
	case 0x00:
		return 1;  /* Return 1: got EOM */
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
		break;
	case 0x10:
		(void) duk__debug_read_uint32_raw(thr);
		break;
	case 0x11:
	case 0x13:
		len = duk__debug_read_uint32_raw(thr);
		duk_debug_skip_bytes(thr, len);
		break;
	case 0x12:
	case 0x14:
		len = duk__debug_read_uint16_raw(thr);
		duk_debug_skip_bytes(thr, len);
		break;
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
		break;
	case 0x1a:
		duk_debug_skip_bytes(thr, 8);
		break;
	case 0x1b:
		duk_debug_skip_byte(thr);
		len = duk_debug_read_byte(thr);
		duk_debug_skip_bytes(thr, len);
		break;
	case 0x1c:
		len = duk_debug_read_byte(thr);
		duk_debug_skip_bytes(thr, len);
		break;
	case 0x1d:
		duk_debug_skip_bytes(thr, 2);
		len = duk_debug_read_byte(thr);
		duk_debug_skip_bytes(thr, len);
		break;
	default:
		goto fail;
	}

	return 0;

 fail:
	DUK__SET_CONN_BROKEN(thr);
	return 1;  /* Pretend like we got EOM */
}

/* Skip dvalues to EOM. */
DUK_LOCAL void duk__debug_skip_to_eom(duk_hthread *thr) {
	for (;;) {
		if (duk__debug_skip_dvalue(thr)) {
			break;
		}
	}
}

/*
 *  Process incoming debug requests
 */

DUK_LOCAL void duk__debug_handle_basic_info(duk_hthread *thr, duk_heap *heap) {
	DUK_UNREF(heap);
	DUK_D(DUK_DPRINT("debug command version"));

	duk_debug_write_reply(thr);
	duk_debug_write_int(thr, DUK_VERSION);
	duk_debug_write_cstring(thr, DUK_GIT_DESCRIBE);
	duk_debug_write_cstring(thr, DUK_USE_TARGET_INFO);
#if defined(DUK_USE_DOUBLE_LE)
	duk_debug_write_int(thr, 1);
#elif defined(DUK_USE_DOUBLE_ME)
	duk_debug_write_int(thr, 2);
#elif defined(DUK_USE_DOUBLE_BE)
	duk_debug_write_int(thr, 3);
#else
	duk_debug_write_int(thr, 0);
#endif
	duk_debug_write_eom(thr);
}

DUK_LOCAL void duk__debug_handle_trigger_status(duk_hthread *thr, duk_heap *heap) {
	DUK_UNREF(heap);
	DUK_D(DUK_DPRINT("debug command triggerstatus"));

	duk_debug_write_reply(thr);
	duk_debug_write_eom(thr);
	heap->dbg_state_dirty = 1;
}

DUK_LOCAL void duk__debug_handle_pause(duk_hthread *thr, duk_heap *heap) {
	DUK_D(DUK_DPRINT("debug command pause"));

	DUK_HEAP_SET_PAUSED(heap);
	duk_debug_write_reply(thr);
	duk_debug_write_eom(thr);
}

DUK_LOCAL void duk__debug_handle_resume(duk_hthread *thr, duk_heap *heap) {
	DUK_D(DUK_DPRINT("debug command resume"));

	DUK_HEAP_CLEAR_PAUSED(heap);
	duk_debug_write_reply(thr);
	duk_debug_write_eom(thr);
}

DUK_LOCAL void duk__debug_handle_step(duk_hthread *thr, duk_heap *heap, duk_int32_t cmd) {
	duk_small_uint_t step_type;
	duk_uint_fast32_t line;

	if (cmd == DUK_DBG_CMD_STEPINTO) {
		step_type = DUK_STEP_TYPE_INTO;
	} else if (cmd == DUK_DBG_CMD_STEPOVER) {
		step_type = DUK_STEP_TYPE_OVER;
	} else {
		DUK_ASSERT(cmd == DUK_DBG_CMD_STEPOUT);
		step_type = DUK_STEP_TYPE_OUT;
	}

	DUK_D(DUK_DPRINT("debug command stepinto/stepover/stepout: %d", (int) cmd));
	line = duk_debug_curr_line(thr);
	if (line > 0) {
		heap->dbg_paused = 0;
		heap->dbg_step_type = step_type;
		heap->dbg_step_thread = thr;
		heap->dbg_step_csindex = thr->callstack_top - 1;
		heap->dbg_step_startline = line;
		heap->dbg_state_dirty = 1;
	} else {
		DUK_D(DUK_DPRINT("cannot determine current line, stepinto/stepover/stepout ignored"));
	}
	duk_debug_write_reply(thr);
	duk_debug_write_eom(thr);
}

DUK_LOCAL void duk__debug_handle_list_break(duk_hthread *thr, duk_heap *heap) {
	duk_small_int_t i;

	DUK_D(DUK_DPRINT("debug command listbreak"));
	duk_debug_write_reply(thr);
	for (i = 0; i < (duk_small_int_t) heap->dbg_breakpoint_count; i++) {
		duk_debug_write_hstring(thr, heap->dbg_breakpoints[i].filename);
		duk_debug_write_uint(thr, (duk_uint32_t) heap->dbg_breakpoints[i].line);
	}
	duk_debug_write_eom(thr);
}

DUK_LOCAL void duk__debug_handle_add_break(duk_hthread *thr, duk_heap *heap) {
	duk_context *ctx = (duk_context *) thr;
	duk_hstring *filename;
	duk_uint32_t linenumber;
	duk_small_int_t idx;

	DUK_UNREF(heap);

	filename = duk_debug_read_hstring(thr);
	linenumber = (duk_uint32_t) duk_debug_read_int(thr);
	DUK_D(DUK_DPRINT("debug command addbreak: %!O:%ld", (duk_hobject *) filename, (long) linenumber));
	idx = duk_debug_add_breakpoint(thr, filename, linenumber);
	if (idx >= 0) {
		duk_debug_write_reply(thr);
		duk_debug_write_int(thr, (duk_int32_t) idx);
		duk_debug_write_eom(thr);
	} else {
		duk_debug_write_error_eom(thr, DUK_DBG_ERR_TOOMANY, "no space for breakpoint");
	}
	duk_pop(ctx);
}

DUK_LOCAL void duk__debug_handle_del_break(duk_hthread *thr, duk_heap *heap) {
	duk_small_uint_t idx;

	DUK_UNREF(heap);

	DUK_D(DUK_DPRINT("debug command delbreak"));
	idx = (duk_small_uint_t) duk_debug_read_int(thr);
	if (duk_debug_remove_breakpoint(thr, idx)) {
		duk_debug_write_reply(thr);
		duk_debug_write_eom(thr);
	} else {
		duk_debug_write_error_eom(thr, DUK_DBG_ERR_NOTFOUND, "invalid breakpoint index");
	}
}

DUK_LOCAL void duk__debug_handle_get_var(duk_hthread *thr, duk_heap *heap) {
	duk_context *ctx = (duk_context *) thr;
	duk_hstring *str;
	duk_bool_t rc;

	DUK_UNREF(heap);
	DUK_D(DUK_DPRINT("debug command getvar"));

	str = duk_debug_read_hstring(thr);  /* push to stack */
	DUK_ASSERT(str != NULL);

	if (thr->callstack_top > 0) {
		rc = duk_js_getvar_activation(thr,
		                              thr->callstack + thr->callstack_top - 1,
		                              str,
		                              0);
	} else {
		/* No activation, no variable access.  Could also pretend
		 * we're in the global program context and read stuff off
		 * the global object.
		 */
		DUK_D(DUK_DPRINT("callstack empty, no activation -> ignore getvar"));
		rc = 0;
	}

	duk_debug_write_reply(thr);
	if (rc) {
		duk_debug_write_int(thr, 1);
		duk_debug_write_tval(thr, duk_require_tval(ctx, -2));
		duk_pop_2(ctx);
	} else {
		duk_debug_write_int(thr, 0);
		duk_debug_write_unused(thr);
	}
	duk_pop(ctx);
	duk_debug_write_eom(thr);
}

DUK_LOCAL void duk__debug_handle_put_var(duk_hthread *thr, duk_heap *heap) {
	duk_context *ctx = (duk_context *) thr;
	duk_hstring *str;
	duk_tval *tv;

	DUK_UNREF(heap);
	DUK_D(DUK_DPRINT("debug command putvar"));

	str = duk_debug_read_hstring(thr);  /* push to stack */
	DUK_ASSERT(str != NULL);
	duk_debug_read_tval(thr);           /* push to stack */
	tv = duk_require_tval(ctx, -1);

	if (thr->callstack_top > 0) {
		duk_js_putvar_activation(thr,
		                         thr->callstack + thr->callstack_top - 1,
		                         str,
		                         tv,
		                         0);
	} else {
		DUK_D(DUK_DPRINT("callstack empty, no activation -> ignore putvar"));
	}
	duk_pop_2(ctx);

	/* XXX: Current putvar implementation doesn't have a success flag,
	 * add one and send to debug client?
	 */
	duk_debug_write_reply(thr);
	duk_debug_write_eom(thr);
}

DUK_LOCAL void duk__debug_handle_get_call_stack(duk_hthread *thr, duk_heap *heap) {
	duk_context *ctx = (duk_context *) thr;
	duk_hthread *curr_thr = thr;
	duk_activation *curr_act;
	duk_uint_fast32_t line;
	duk_size_t i;

	DUK_UNREF(heap);

	duk_debug_write_reply(thr);
	while (curr_thr != NULL) {
		i = curr_thr->callstack_top;
		while (i > 0) {
			i--;
			curr_act = curr_thr->callstack + i;

			/* XXX: optimize to use direct reads,
			 * i.e. avoid value stack operations.
			 */
			duk_push_tval(ctx, &curr_act->tv_func);
			duk_get_prop_stridx(ctx, -1, DUK_STRIDX_FILE_NAME);
			duk_safe_to_string(ctx, -1);
			duk_debug_write_hstring(thr, duk_get_hstring(ctx, -1));
			duk_get_prop_stridx(ctx, -2, DUK_STRIDX_NAME);
			duk_safe_to_string(ctx, -1);
			duk_debug_write_hstring(thr, duk_get_hstring(ctx, -1));
			line = duk_hobject_pc2line_query(ctx, -3, curr_act->pc);
			duk_debug_write_uint(thr, (duk_uint32_t) line);
			duk_debug_write_uint(thr, (duk_uint32_t) curr_act->pc);
			duk_pop_3(ctx);
		}
		curr_thr = curr_thr->resumer;
	}
	duk_debug_write_eom(thr);
}

DUK_LOCAL void duk__debug_handle_get_locals(duk_hthread *thr, duk_heap *heap) {
	duk_context *ctx = (duk_context *) thr;
	duk_activation *curr_act;
	duk_hstring *varname;

	DUK_UNREF(heap);

	duk_debug_write_reply(thr);
	if (thr->callstack_top == 0) {
		goto callstack_empty;
	}
	curr_act = thr->callstack + thr->callstack_top - 1;

	/* XXX: several nice-to-have improvements here:
	 *   - Use direct reads avoiding value stack operations
	 *   - Avoid triggering getters, indicate getter values to debug client
	 *   - If side effects are possible, add error catching
	 */

	duk_push_tval(ctx, &curr_act->tv_func);
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VARMAP);
	if (duk_is_object(ctx, -1)) {
		duk_enum(ctx, -1, 0 /*enum_flags*/);
		while (duk_next(ctx, -1 /*enum_index*/, 0 /*get_value*/)) {
			varname = duk_get_hstring(ctx, -1);
			DUK_ASSERT(varname != NULL);

			duk_js_getvar_activation(thr, curr_act, varname, 0 /*throw_flag*/);
			/* [ ... func varmap enum key value this ] */
			duk_debug_write_hstring(thr, duk_get_hstring(ctx, -3));
			duk_debug_write_tval(thr, duk_get_tval(ctx, -2));
			duk_pop_3(ctx);  /* -> [ ... func varmap enum ] */
		}
		duk_pop(ctx);
	} else {
		DUK_D(DUK_DPRINT("varmap is not an object in GetLocals, ignore"));
	}
	duk_pop_2(ctx);

 callstack_empty:
	duk_debug_write_eom(thr);
}

DUK_LOCAL void duk__debug_handle_eval(duk_hthread *thr, duk_heap *heap) {
	duk_context *ctx = (duk_context *) thr;

	duk_small_uint_t call_flags;
	duk_int_t call_ret;
	duk_small_int_t eval_err;
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif

	DUK_UNREF(heap);

	DUK_D(DUK_DPRINT("debug command eval"));

	/* The eval code must be executed within the current (topmost)
	 * activation.  For now, use global object eval() function, with
	 * the eval considered a 'direct call to eval'.
	 */

#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(ctx);
#endif

	duk_push_c_function(ctx, duk_bi_global_object_eval, 1 /*nargs*/);
	duk_push_undefined(ctx);  /* 'this' binding shouldn't matter here */
	(void) duk_debug_read_hstring(thr);

	/* [ ... eval "eval" eval_input ] */

	call_flags = DUK_CALL_FLAG_PROTECTED;
	if (thr->callstack_top >= 1) {
		duk_activation *act;
		duk_hobject *fun;

		act = thr->callstack + thr->callstack_top - 1;
		fun = DUK_ACT_GET_FUNC(act);
		if (fun && DUK_HOBJECT_IS_COMPILEDFUNCTION(fun)) {
			/* Direct eval requires that there's a current
			 * activation and it is an Ecmascript function.
			 * When Eval is executed from e.g. cooperate API
			 * call we'll need to an indirect eval instead.
			 */
			call_flags |= DUK_CALL_FLAG_DIRECT_EVAL;
		}
	}

	call_ret = duk_handle_call(thr, 1 /*num_stack_args*/, call_flags);

	if (call_ret == DUK_EXEC_SUCCESS) {
		eval_err = 0;
		/* Use result value as is. */
	} else {
		/* For errors a string coerced result is most informative
		 * right now, as the debug client doesn't have the capability
		 * to traverse the error object.
		 */
		eval_err = 1;
		duk_safe_to_string(ctx, -1);
	}

	/* [ ... result ] */

	duk_debug_write_reply(thr);
	duk_debug_write_int(thr, (duk_int32_t) eval_err);
	duk_debug_write_tval(thr, duk_require_tval(ctx, -1));
	duk_debug_write_eom(thr);
	duk_pop(ctx);

	DUK_ASSERT(duk_get_top(ctx) == entry_top);
}

DUK_LOCAL void duk__debug_handle_detach(duk_hthread *thr, duk_heap *heap) {
	DUK_UNREF(heap);
	DUK_D(DUK_DPRINT("debug command detach"));

	duk_debug_write_reply(thr);
	duk_debug_write_eom(thr);

	DUK_D(DUK_DPRINT("debug connection detached, mark broken"));
	DUK__SET_CONN_BROKEN(thr);
}

#if defined(DUK_USE_DEBUGGER_DUMPHEAP)
DUK_LOCAL void duk__debug_dump_heaphdr(duk_hthread *thr, duk_heap *heap, duk_heaphdr *hdr) {
	DUK_UNREF(heap);

	duk_debug_write_heapptr(thr, hdr);
	duk_debug_write_uint(thr, (duk_uint32_t) DUK_HEAPHDR_GET_TYPE(hdr));
	duk_debug_write_uint(thr, (duk_uint32_t) DUK_HEAPHDR_GET_FLAGS_RAW(hdr));
#if defined(DUK_USE_REFERENCE_COUNTING)
	duk_debug_write_uint(thr, (duk_uint32_t) DUK_HEAPHDR_GET_REFCOUNT(hdr));
#else
	duk_debug_write_int(thr, (duk_int32_t) -1);
#endif

	switch (DUK_HEAPHDR_GET_TYPE(hdr)) {
	case DUK_HTYPE_STRING: {
		duk_hstring *h = (duk_hstring *) hdr;

		duk_debug_write_uint(thr, (duk_int32_t) DUK_HSTRING_GET_BYTELEN(h));
		duk_debug_write_uint(thr, (duk_int32_t) DUK_HSTRING_GET_CHARLEN(h));
		duk_debug_write_uint(thr, (duk_int32_t) DUK_HSTRING_GET_HASH(h));
		duk_debug_write_hstring(thr, h);
		break;
	}
	case DUK_HTYPE_OBJECT: {
		duk_hobject *h = (duk_hobject *) hdr;
		duk_hstring *k;
		duk_uint_fast32_t i;

		duk_debug_write_uint(thr, (duk_uint32_t) DUK_HOBJECT_GET_CLASS_NUMBER(h));
		duk_debug_write_heapptr(thr, (duk_heaphdr *) DUK_HOBJECT_GET_PROTOTYPE(heap, h));
		duk_debug_write_uint(thr, (duk_uint32_t) DUK_HOBJECT_GET_ESIZE(h));
		duk_debug_write_uint(thr, (duk_uint32_t) DUK_HOBJECT_GET_ENEXT(h));
		duk_debug_write_uint(thr, (duk_uint32_t) DUK_HOBJECT_GET_ASIZE(h));
		duk_debug_write_uint(thr, (duk_uint32_t) DUK_HOBJECT_GET_HSIZE(h));

		for (i = 0; i < (duk_uint_fast32_t) DUK_HOBJECT_GET_ENEXT(h); i++) {
			duk_debug_write_uint(thr, (duk_uint32_t) DUK_HOBJECT_E_GET_FLAGS(heap, h, i));
			k = DUK_HOBJECT_E_GET_KEY(heap, h, i);
			duk_debug_write_heapptr(thr, (duk_heaphdr *) k);
			if (k == NULL) {
				duk_debug_write_int(thr, 0);  /* isAccessor */
				duk_debug_write_unused(thr);
				continue;
			}
			if (DUK_HOBJECT_E_SLOT_IS_ACCESSOR(heap, h, i)) {
				duk_debug_write_int(thr, 1);  /* isAccessor */
				duk_debug_write_heapptr(thr, (duk_heaphdr *) DUK_HOBJECT_E_GET_VALUE_PTR(heap, h, i)->a.get);
				duk_debug_write_heapptr(thr, (duk_heaphdr *) DUK_HOBJECT_E_GET_VALUE_PTR(heap, h, i)->a.set);
			} else {
				duk_debug_write_int(thr, 0);  /* isAccessor */

				duk__debug_write_tval_heapptr(thr, &DUK_HOBJECT_E_GET_VALUE_PTR(heap, h, i)->v);
			}
		}

		for (i = 0; i < (duk_uint_fast32_t) DUK_HOBJECT_GET_ASIZE(h); i++) {
			/* Note: array dump will include elements beyond
			 * 'length'.
			 */
			duk__debug_write_tval_heapptr(thr, DUK_HOBJECT_A_GET_VALUE_PTR(heap, h, i));
		}
		break;
	}
	case DUK_HTYPE_BUFFER: {
		duk_hbuffer *h = (duk_hbuffer *) hdr;

		duk_debug_write_uint(thr, (duk_uint32_t) DUK_HBUFFER_GET_SIZE(h));
		if (DUK_HBUFFER_HAS_DYNAMIC(h)) {
			duk_debug_write_uint(thr, (duk_uint32_t) DUK_HBUFFER_DYNAMIC_GET_ALLOC_SIZE((duk_hbuffer_dynamic *) h));
		} else {
			duk_debug_write_uint(thr, (duk_uint32_t) DUK_HBUFFER_GET_SIZE(h));
		}
		duk_debug_write_buffer(thr, (const char *) DUK_HBUFFER_GET_DATA_PTR(heap, h), (duk_size_t) DUK_HBUFFER_GET_SIZE(h));
		break;
	}
	default: {
		DUK_D(DUK_DPRINT("invalid htype: %d", (int) DUK_HEAPHDR_GET_TYPE(hdr)));
	}
	}
}

DUK_LOCAL void duk__debug_dump_heap_allocated(duk_hthread *thr, duk_heap *heap) {
	duk_heaphdr *hdr;

	hdr = heap->heap_allocated;
	while (hdr != NULL) {
		duk__debug_dump_heaphdr(thr, heap, hdr);
		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}
}

#if defined(DUK_USE_STRTAB_CHAIN)
DUK_LOCAL void duk__debug_dump_strtab_chain(duk_hthread *thr, duk_heap *heap) {
	duk_uint_fast32_t i, j;
	duk_strtab_entry *e;
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t *lst;
#else
	duk_hstring **lst;
#endif
	duk_hstring *h;

	for (i = 0; i < DUK_STRTAB_CHAIN_SIZE; i++) {
		e = heap->strtable + i;
		if (e->listlen > 0) {
#if defined(DUK_USE_HEAPPTR16)
			lst = (duk_uint16_t *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, e->u.strlist16);
#else
			lst = e->u.strlist;
#endif
			DUK_ASSERT(lst != NULL);

			for (j = 0; j < e->listlen; j++) {
#if defined(DUK_USE_HEAPPTR16)
				h = DUK_USE_HEAPPTR_DEC16(heap->heap_udata, lst[j]);
#else
				h = lst[j];
#endif
				if (h != NULL) {
					duk__debug_dump_heaphdr(thr, heap, (duk_heaphdr *) h);
				}
			}
		} else {
#if defined(DUK_USE_HEAPPTR16)
			h = DUK_USE_HEAPPTR_DEC16(heap->heap_udata, e->u.str16);
#else
			h = e->u.str;
#endif
			if (h != NULL) {
				duk__debug_dump_heaphdr(thr, heap, (duk_heaphdr *) h);
			}
		}
	}
}
#endif  /* DUK_USE_STRTAB_CHAIN */

#if defined(DUK_USE_STRTAB_PROBE)
DUK_LOCAL void duk__debug_dump_strtab_probe(duk_hthread *thr, duk_heap *heap) {
	duk_uint32_t i;
	duk_hstring *h;

	for (i = 0; i < heap->st_size; i++) {
#if defined(DUK_USE_HEAPPTR16)
		h = DUK_USE_HEAPPTR_DEC16(heap->strtable16[i]);
#else
		h = heap->strtable[i];
#endif
		if (h == NULL || h == DUK_STRTAB_DELETED_MARKER(heap)) {
			continue;
		}

		duk__debug_dump_heaphdr(thr, heap, (duk_heaphdr *) h);
	}
}
#endif  /* DUK_USE_STRTAB_PROBE */

DUK_LOCAL void duk__debug_handle_dump_heap(duk_hthread *thr, duk_heap *heap) {
	DUK_D(DUK_DPRINT("debug command dumpheap"));

	duk_debug_write_reply(thr);
	duk__debug_dump_heap_allocated(thr, heap);
#if defined(DUK_USE_STRTAB_CHAIN)
	duk__debug_dump_strtab_chain(thr, heap);
#endif
#if defined(DUK_USE_STRTAB_PROBE)
	duk__debug_dump_strtab_probe(thr, heap);
#endif
	duk_debug_write_eom(thr);
}
#endif  /* DUK_USE_DEBUGGER_DUMPHEAP */

DUK_LOCAL void duk__debug_handle_get_bytecode(duk_hthread *thr, duk_heap *heap) {
	duk_activation *act;
	duk_hcompiledfunction *fun;
	duk_size_t i, n;
	duk_tval *tv;
	duk_hobject **fn;

	DUK_UNREF(heap);

	DUK_D(DUK_DPRINT("debug command getbytecode"));

	duk_debug_write_reply(thr);
	if (thr->callstack_top == 0) {
		fun = NULL;
	} else {
		act = thr->callstack + thr->callstack_top - 1;
		fun = (duk_hcompiledfunction *) DUK_ACT_GET_FUNC(act);
		if (!DUK_HOBJECT_IS_COMPILEDFUNCTION((duk_hobject *) fun)) {
			fun = NULL;
		}
	}
	DUK_ASSERT(fun == NULL || DUK_HOBJECT_IS_COMPILEDFUNCTION((duk_hobject *) fun));

	if (fun) {
		n = DUK_HCOMPILEDFUNCTION_GET_CONSTS_COUNT(heap, fun);
		duk_debug_write_int(thr, (int) n);
		tv = DUK_HCOMPILEDFUNCTION_GET_CONSTS_BASE(heap, fun);
		for (i = 0; i < n; i++) {
			duk_debug_write_tval(thr, tv);
			tv++;
		}

		n = DUK_HCOMPILEDFUNCTION_GET_FUNCS_COUNT(heap, fun);
		duk_debug_write_int(thr, (int) n);
		fn = DUK_HCOMPILEDFUNCTION_GET_FUNCS_BASE(heap, fun);
		for (i = 0; i < n; i++) {
			duk_debug_write_hobject(thr, *fn);
			fn++;
		}

		duk_debug_write_string(thr,
		                       (const char *) DUK_HCOMPILEDFUNCTION_GET_CODE_BASE(heap, fun),
		                       (duk_size_t) DUK_HCOMPILEDFUNCTION_GET_CODE_SIZE(heap, fun));
	} else {
		duk_debug_write_int(thr, 0);
		duk_debug_write_int(thr, 0);
		duk_debug_write_cstring(thr, "");
	}
	duk_debug_write_eom(thr);
}

DUK_LOCAL void duk__debug_process_message(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
	duk_heap *heap;
	duk_uint8_t x;
	duk_int32_t cmd;

	DUK_ASSERT(thr != NULL);
	heap = thr->heap;
	DUK_ASSERT(heap != NULL);
	DUK_UNREF(ctx);

	x = duk_debug_read_byte(thr);
	switch (x) {
	case DUK_DBG_MARKER_REQUEST: {
		cmd = duk_debug_read_int(thr);
		switch (cmd) {
		case DUK_DBG_CMD_BASICINFO: {
			duk__debug_handle_basic_info(thr, heap);
			break;
		}
		case DUK_DBG_CMD_TRIGGERSTATUS: {
			duk__debug_handle_trigger_status(thr, heap);
			break;
		}
		case DUK_DBG_CMD_PAUSE: {
			duk__debug_handle_pause(thr, heap);
			break;
		}
		case DUK_DBG_CMD_RESUME: {
			duk__debug_handle_resume(thr, heap);
			break;
		}
		case DUK_DBG_CMD_STEPINTO:
		case DUK_DBG_CMD_STEPOVER:
		case DUK_DBG_CMD_STEPOUT: {
			duk__debug_handle_step(thr, heap, cmd);
			break;
		}
		case DUK_DBG_CMD_LISTBREAK: {
			duk__debug_handle_list_break(thr, heap);
			break;
		}
		case DUK_DBG_CMD_ADDBREAK: {
			duk__debug_handle_add_break(thr, heap);
			break;
		}
		case DUK_DBG_CMD_DELBREAK: {
			duk__debug_handle_del_break(thr, heap);
			break;
		}
		case DUK_DBG_CMD_GETVAR: {
			duk__debug_handle_get_var(thr, heap);
			break;
		}
		case DUK_DBG_CMD_PUTVAR: {
			duk__debug_handle_put_var(thr, heap);
			break;
		}
		case DUK_DBG_CMD_GETCALLSTACK: {
			duk__debug_handle_get_call_stack(thr, heap);
			break;
		}
		case DUK_DBG_CMD_GETLOCALS: {
			duk__debug_handle_get_locals(thr, heap);
			break;
		}
		case DUK_DBG_CMD_EVAL: {
			duk__debug_handle_eval(thr, heap);
			break;
		}
		case DUK_DBG_CMD_DETACH: {
			duk__debug_handle_detach(thr, heap);
			break;
		}
#if defined(DUK_USE_DEBUGGER_DUMPHEAP)
		case DUK_DBG_CMD_DUMPHEAP: {
			duk__debug_handle_dump_heap(thr, heap);
			break;
		}
#endif  /* DUK_USE_DEBUGGER_DUMPHEAP */
		case DUK_DBG_CMD_GETBYTECODE: {
			duk__debug_handle_get_bytecode(thr, heap);
			break;
		}
		default: {
			DUK_D(DUK_DPRINT("debug command unsupported: %d", (int) cmd));
			duk_debug_write_error_eom(thr, DUK_DBG_ERR_UNSUPPORTED, "unsupported command");
		}
		}  /* switch cmd */
		break;
	}
	case DUK_DBG_MARKER_REPLY: {
		DUK_D(DUK_DPRINT("debug reply, skipping"));
		break;
	}
	case DUK_DBG_MARKER_ERROR: {
		DUK_D(DUK_DPRINT("debug error, skipping"));
		break;
	}
	case DUK_DBG_MARKER_NOTIFY: {
		DUK_D(DUK_DPRINT("debug notify, skipping"));
		break;
	}
	default: {
		DUK_D(DUK_DPRINT("invalid initial byte, drop connection: %d", (int) x));
		goto fail;
	}
	}  /* switch initial byte */

	duk__debug_skip_to_eom(thr);
	return;

 fail:
	DUK__SET_CONN_BROKEN(thr);
	return;
}

DUK_INTERNAL duk_bool_t duk_debug_process_messages(duk_hthread *thr, duk_bool_t no_block) {
	duk_context *ctx = (duk_context *) thr;
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif
	duk_bool_t retval = 0;

	DUK_ASSERT(thr != NULL);
	DUK_UNREF(ctx);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(ctx);
#endif

	DUK_DD(DUK_DDPRINT("top at entry: %ld", (long) duk_get_top(ctx)));

	for (;;) {
		/* Process messages until we're no longer paused or we peek
		 * and see there's nothing to read right now.
		 */
		DUK_DD(DUK_DDPRINT("top at loop top: %ld", (long) duk_get_top(ctx)));

		if (thr->heap->dbg_read_cb == NULL) {
			DUK_D(DUK_DPRINT("debug connection broken, stop processing messages"));
			break;
		} else if (!thr->heap->dbg_paused || no_block) {
			if (!duk_debug_read_peek(thr)) {
				DUK_D(DUK_DPRINT("processing debug message, peek indicated no data, stop processing"));
				break;
			}
			DUK_D(DUK_DPRINT("processing debug message, peek indicated there is data, handle it"));
		} else {
			DUK_D(DUK_DPRINT("paused, process debug message, blocking if necessary"));
		}

		duk__debug_process_message(thr);
		if (thr->heap->dbg_state_dirty) {
			/* Executed something that may have affected status,
			 * resend.
			 */
			duk_debug_send_status(thr);
			thr->heap->dbg_state_dirty = 0;
		}
		retval = 1;  /* processed one or more messages */
	}

	/* As an initial implementation, read flush after exiting the message
	 * loop.
	 */
	duk_debug_read_flush(thr);

	DUK_DD(DUK_DDPRINT("top at exit: %ld", (long) duk_get_top(ctx)));

#if defined(DUK_USE_ASSERTIONS)
	/* Easy to get wrong, so assert for it. */
	DUK_ASSERT(entry_top == duk_get_top(ctx));
#endif

	return retval;
}

/*
 *  Breakpoint management
 */

DUK_INTERNAL duk_small_int_t duk_debug_add_breakpoint(duk_hthread *thr, duk_hstring *filename, duk_uint32_t line) {
	duk_heap *heap;
	duk_breakpoint *b;

	/* Caller must trigger recomputation of active breakpoint list.  To
	 * ensure stale values are not used if that doesn't happen, clear the
	 * active breakpoint list here.
	 */

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(filename != NULL);

	heap = thr->heap;
	if (heap->dbg_breakpoint_count >= DUK_HEAP_MAX_BREAKPOINTS) {
		DUK_D(DUK_DPRINT("failed to add breakpoint for %O:%ld, all breakpoint slots used",
		                 (duk_heaphdr *) filename, (long) line));
		return -1;
	}
	heap->dbg_breakpoints_active[0] = (duk_breakpoint *) NULL;
	b = heap->dbg_breakpoints + (heap->dbg_breakpoint_count++);
	b->filename = filename;
	b->line = line;
	DUK_HSTRING_INCREF(thr, filename);

	return heap->dbg_breakpoint_count - 1;  /* index */
}

DUK_INTERNAL duk_bool_t duk_debug_remove_breakpoint(duk_hthread *thr, duk_small_uint_t breakpoint_index) {
	duk_heap *heap = thr->heap;
	duk_hstring *h;
	duk_breakpoint *b;
	duk_size_t move_size;

	/* Caller must trigger recomputation of active breakpoint list.  To
	 * ensure stale values are not used if that doesn't happen, clear the
	 * active breakpoint list here.
	 */

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(breakpoint_index >= 0);  /* unsigned */

	if (breakpoint_index >= heap->dbg_breakpoint_count) {
		DUK_D(DUK_DPRINT("invalid breakpoint index: %ld", (long) breakpoint_index));
		return 0;
	}
	b = heap->dbg_breakpoints + breakpoint_index;

	h = b->filename;
	DUK_ASSERT(h != NULL);

	move_size = sizeof(duk_breakpoint) * (heap->dbg_breakpoint_count - breakpoint_index - 1);
	if (move_size > 0) {
		DUK_MEMMOVE((void *) b,
		            (void *) (b + 1),
		            move_size);
	}
	heap->dbg_breakpoint_count--;
	heap->dbg_breakpoints_active[0] = (duk_breakpoint *) NULL;

	DUK_HSTRING_DECREF(thr, h);  /* side effects */

	/* Breakpoint entries above the used area are left as garbage. */

	return 1;
}

#undef DUK__SET_CONN_BROKEN

#else  /* DUK_USE_DEBUGGER_SUPPORT */

/* No debugger support. */

#endif  /* DUK_USE_DEBUGGER_SUPPORT */
