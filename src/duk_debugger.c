/*
 *  Duktape debugger
 */

#include "duk_internal.h"

#if defined(DUK_USE_DEBUGGER_SUPPORT)

/*
 *  Debug connection helpers
 */

#define DUK__SET_CONN_BROKEN(heap) do { \
		duk__debug_set_broken((heap)); \
	} while (0)

DUK_LOCAL void duk__debug_set_broken(duk_heap *heap) {
	heap->dbg_read_cb = NULL;
	heap->dbg_write_cb = NULL;
}

/*
 *  Debug connection peek and flush primitives
 */

DUK_INTERNAL duk_bool_t duk_debug_read_peek(duk_hthread *thr) {
	duk_heap *heap;

	DUK_ASSERT(thr != NULL);
	heap = thr->heap;

	if (heap->dbg_read_cb == NULL) {
		DUK_D(DUK_DPRINT("attempt to peek in error state, return zero (= no data)"));
		return 0;
	}

	return (duk_bool_t) heap->dbg_read_cb(heap->dbg_udata, NULL, 0);
}

DUK_INTERNAL void duk_debug_write_flush(duk_hthread *thr) {
	duk_heap *heap;

	DUK_ASSERT(thr != NULL);
	heap = thr->heap;

	if (heap->dbg_write_cb == NULL) {
		DUK_D(DUK_DPRINT("attempt to write flush in error state, ignore"));
		return;
	}

	heap->dbg_write_cb(heap->dbg_udata, (const char *) NULL, (duk_size_t) 0);
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
		DUK_D(DUK_DPRINT("attempt to read %ld bytes in error state, return zero data", (long) length));
		goto fail;
	}

	p = data;
	for (;;) {
		left = (duk_size_t) ((data + length) - p);
		if (left == 0) {
			break;
		}
		DUK_ASSERT(heap->dbg_read_cb != NULL);
		got = heap->dbg_read_cb(heap->dbg_udata, (char *) p, left);
		if (got == 0 || got > left) {
			DUK_D(DUK_DPRINT("connection error during read, return zero data"));
			DUK__SET_CONN_BROKEN(heap);
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
		DUK_D(DUK_DPRINT("attempt to read 1 bytes in error state, return zero data"));
		return 0;
	}

	x = 0;  /* just in case callback is broken and won't write 'x' */
	DUK_ASSERT(heap->dbg_read_cb != NULL);
	got = heap->dbg_read_cb(heap->dbg_udata, (char *) (&x), 1);
	if (got != 1) {
		DUK_D(DUK_DPRINT("connection error during read, return zero data"));
		DUK__SET_CONN_BROKEN(heap);
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
	DUK__SET_CONN_BROKEN(thr->heap);
	return 0;
}

DUK_INTERNAL duk_hstring *duk__debug_read_hstring_raw(duk_hthread *thr, duk_uint32_t len) {
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
	DUK__SET_CONN_BROKEN(thr->heap);
	duk_push_hstring_stridx(thr, DUK_STRIDX_EMPTY_STRING);  /* always push some string */
	return duk_require_hstring(ctx, -1);
}

DUK_INTERNAL const void *duk_debug_read_pointer(duk_hthread *thr) {
	duk_uint8_t buf[2];
	volatile void *ptr;

	DUK_ASSERT(thr != NULL);

	/* FIXME: endianness */
	/* FIXME: union */
	duk_debug_read_bytes(thr, buf, 2);
	if (buf[0] != 0x1c || buf[1] != sizeof(ptr)) {
		goto fail;
	}
	duk_debug_read_bytes(thr, (duk_uint8_t *) &ptr, sizeof(ptr));
	return (const void *) ptr;

 fail:
	DUK_D(DUK_DPRINT("debug connection error: failed to decode pointer"));
	DUK__SET_CONN_BROKEN(thr->heap);
	return (const void *) NULL;
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
		duk_push_number(ctx, (duk_double_t) i);
		break;
	}
	case 0x11:
		/* FIXME */
		break;
	case 0x12:
		/* FIXME */
		break;
	case 0x13:
		/* FIXME */
		break;
	case 0x14:
		/* FIXME */
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
	case 0x1a:
		/* FIXME: number */
		goto fail;
		break;
	case 0x1b:
		/* FIXME: object */
		goto fail;
		break;
	case 0x1c: {
		const void *ptr;
		ptr = duk_debug_read_pointer(thr);
		duk_push_pointer(thr, (void *) ptr);
		break;
	}
	case 0x1d:
		/* FIXME: lightfunc */
		goto fail;
		break;
	default:
		goto fail;
	}

	return;

 fail:
	DUK_D(DUK_DPRINT("debug connection error: failed to decode tval"));
	DUK__SET_CONN_BROKEN(thr->heap);
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
		DUK_D(DUK_DPRINT("attempt to write %ld bytes in error state, ignore", (long) length));
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
		got = heap->dbg_write_cb(heap->dbg_udata, (const char *) p, left);
		if (got == 0 || got > left) {
			DUK_D(DUK_DPRINT("connection error during write"));
			DUK__SET_CONN_BROKEN(heap);
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
		DUK_D(DUK_DPRINT("attempt to write 1 bytes in error state, ignore"));
		return;
	}

	DUK_ASSERT(heap->dbg_write_cb != NULL);
	got = heap->dbg_write_cb(heap->dbg_udata, (const char *) (&x), 1);
	if (got != 1) {
		DUK_D(DUK_DPRINT("connection error during write"));
		DUK__SET_CONN_BROKEN(heap);
	}
}

/* Write signed 32-bit integer.  There's currently no need to support full
 * 32-bit unsigned integer range in practice.
 */
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

DUK_INTERNAL void duk_debug_write_pointer(duk_hthread *thr, const void *ptr) {
	duk_uint8_t buf[2];

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(sizeof(ptr) >= 1 && sizeof(ptr) <= 16);

	/* FIXME: endianness */
	/* FIXME: union */
	buf[0] = 0x1c;
	buf[1] = sizeof(ptr);
	duk_debug_write_bytes(thr, buf, 2);
	duk_debug_write_bytes(thr, (const duk_uint8_t *) &ptr, (duk_size_t) sizeof(ptr));
}

DUK_INTERNAL void duk_debug_write_heapptr(duk_hthread *thr, duk_heaphdr *h) {
	duk_uint8_t buf[2];

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(sizeof(h) >= 1 && sizeof(h) <= 16);

	/* FIXME: endianness */
	/* FIXME: union */
	buf[0] = 0x1e;
	buf[1] = sizeof(h);
	duk_debug_write_bytes(thr, buf, 2);
	duk_debug_write_bytes(thr, (const duk_uint8_t *) &h, (duk_size_t) sizeof(h));
}

DUK_INTERNAL void duk_debug_write_hobject(duk_hthread *thr, duk_hobject *obj) {
	duk_uint8_t buf[3];

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(sizeof(obj) >= 1 && sizeof(obj) <= 16);
	/* obj may be NULL */

	/* FIXME: endianness */
	/* FIXME: union */
	buf[0] = 0x1b;
	buf[1] = (duk_uint8_t) (obj ? DUK_HOBJECT_GET_CLASS_NUMBER(obj) : 0);
	buf[2] = sizeof(obj);
	duk_debug_write_bytes(thr, buf, 3);
	duk_debug_write_bytes(thr, (const duk_uint8_t *) &obj, (duk_size_t) sizeof(obj));
}

DUK_INTERNAL void duk_debug_write_unused(duk_hthread *thr) {
	duk_debug_write_byte(thr, 0x15);
}

DUK_INTERNAL void duk_debug_write_tval(duk_hthread *thr, duk_tval *tv) {
	duk_c_function lf_func;
	duk_small_uint_t lf_flags;
	duk_uint8_t buf[3];
	duk_double_t d;

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
		duk_debug_write_bytes(thr, buf, 3);
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
	default:
		/* Numbers are normalized to big (network) endian. */

		/* FIXME: implement */
#if defined(DUK_USE_DOUBLE_LE)
#elif defined(DUK_USE_DOUBLE_ME)
#elif defined(DUK_USE_DOUBLE_BE)
#else
#error internal error, double endianness insane
#endif

		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		d = DUK_TVAL_GET_NUMBER(tv);
		duk_debug_write_byte(thr, 0x1a);
		duk_debug_write_bytes(thr, (const duk_uint8_t *) &d, sizeof(d));  /* FIXME: dblunion */
	}
}

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

	act = thr->callstack + thr->callstack_top - 1;

	duk_debug_write_notify(thr, DUK_DBG_CMD_STATUS);
	duk_debug_write_int(thr, thr->heap->dbg_paused);
	duk_push_hobject(ctx, act->func);
	duk_get_prop_string(ctx, -1, "fileName");
	duk_safe_to_string(ctx, -1);
	duk_debug_write_hstring(thr, duk_require_hstring(ctx, -1));
	duk_get_prop_string(ctx, -2, "name");
	duk_safe_to_string(ctx, -1);
	duk_debug_write_hstring(thr, duk_require_hstring(ctx, -1));
	duk_pop_3(ctx);
	duk_debug_write_int(thr, (duk_int32_t) duk_debug_curr_line(thr));
	duk_debug_write_int(thr, (duk_int32_t) act->pc);
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
	DUK__SET_CONN_BROKEN(thr->heap);
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

	heap->dbg_paused = 1;
	DUK_HEAP_CLEAR_STEP_STATE(heap);
	heap->dbg_state_dirty = 1;
	duk_debug_write_reply(thr);
	duk_debug_write_eom(thr);
}

DUK_LOCAL void duk__debug_handle_resume(duk_hthread *thr, duk_heap *heap) {
	DUK_D(DUK_DPRINT("debug command resume"));

	heap->dbg_paused = 0;
	DUK_HEAP_CLEAR_STEP_STATE(heap);
	heap->dbg_state_dirty = 1;
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
		duk_debug_write_int(thr, (duk_int32_t) heap->dbg_breakpoints[i].line);
	}
	duk_debug_write_eom(thr);
}

DUK_LOCAL void duk__debug_handle_add_break(duk_hthread *thr, duk_heap *heap) {
	duk_context *ctx = (duk_context *) thr;
	duk_hstring *filename;
	duk_uint32_t linenumber;
	duk_small_int_t idx;

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
	heap->dbg_brkpt_dirty = 1;
}

DUK_LOCAL void duk__debug_handle_del_break(duk_hthread *thr, duk_heap *heap) {
	duk_small_uint_t idx;

	DUK_D(DUK_DPRINT("debug command delbreak"));
	idx = (duk_small_uint_t) duk_debug_read_int(thr);
	duk_debug_remove_breakpoint(thr, idx);
	duk_debug_write_reply(thr);
	duk_debug_write_eom(thr);
	heap->dbg_brkpt_dirty = 1;
}

DUK_LOCAL void duk__debug_handle_get_var(duk_hthread *thr, duk_heap *heap) {
	duk_context *ctx = (duk_context *) thr;
	duk_hstring *str;
	duk_bool_t rc;

	DUK_UNREF(heap);
	DUK_D(DUK_DPRINT("debug command getvar"));

	str = duk_debug_read_hstring(thr);  /* push to stack */
	DUK_ASSERT(str != NULL);

	rc = duk_js_getvar_activation(thr,
	                              thr->callstack + thr->callstack_top - 1,
	                              str,
	                              0);

	duk_debug_write_reply(thr);
	if (rc) {
		duk_debug_write_int(thr, 1);
		duk_debug_write_tval(thr, duk_require_tval(ctx, -2));
		duk_pop_2(ctx);
	} else {
		duk_debug_write_int(thr, 0);
		duk_push_unused(thr);
		duk_debug_write_tval(thr, duk_require_tval(ctx, -1));
		duk_pop(ctx);
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

	duk_js_putvar_activation(thr,
	                         thr->callstack + thr->callstack_top - 1,
	                         str,
	                         tv,
	                         0);

	duk_pop_2(ctx);

	duk_debug_write_reply(thr);
	/* FIXME: success flag */
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
			duk_debug_write_int(thr, (duk_int32_t) line);
			duk_debug_write_int(thr, (duk_int32_t) curr_act->pc);
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

	/* XXX: optimize to use direct reads, i.e. avoid
	 * value stack operations.
	 */
	/* FIXME: error handling */
	duk_push_tval(ctx, &curr_act->tv_func);
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VARMAP);
	if (duk_is_object(ctx, -1)) {
		duk_enum(ctx, -1, 0 /*enum_flags*/);
		while (duk_next(ctx, -1 /*enum_index*/, 0 /*get_value*/)) {
			/* FIXME: dealing with values properly (avoid getters etc) */
			varname = duk_get_hstring(ctx, -1);
			DUK_ASSERT(varname != NULL);

			/* FIXME: the danger here is that we enter a getter
			 * or cause some other side effect which causes debug
			 * communication before our message is finished.  A
			 * getter might, for instance, print() which gets
			 * forwarded and garbles the debug stream.
			 */
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

	/* FIXME: it's probably best to remove dbg_brkpt_dirty and just recheck
	 * breakpoints whenever any debug commands were executed?
	 */

	DUK_UNREF(heap);

	duk_small_uint_t call_flags;
	duk_int_t call_ret;
	duk_small_int_t eval_err;
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif

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

	call_flags = DUK_CALL_FLAG_DIRECT_EVAL | DUK_CALL_FLAG_PROTECTED;

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

	heap->dbg_brkpt_dirty = 1;  /* FIXME: need to recheck before returning from interrupt? */
}

DUK_LOCAL void duk__debug_handle_detach(duk_hthread *thr, duk_heap *heap) {
	DUK_UNREF(heap);
	DUK_D(DUK_DPRINT("debug command detach"));

	duk_debug_write_error_eom(thr, DUK_DBG_ERR_UNSUPPORTED, "detach unimplemented");  /*FIXME*/
}

#if defined(DUK_USE_DEBUGGER_DUMPHEAP)
DUK_LOCAL void duk__debug_dump_heaphdr(duk_hthread *thr, duk_heap *heap, duk_heaphdr *hdr) {
	DUK_UNREF(heap);

	duk_debug_write_heapptr(thr, hdr);
	duk_debug_write_int(thr, (duk_int32_t) DUK_HEAPHDR_GET_TYPE(hdr));
	duk_debug_write_int(thr, (duk_int32_t) DUK_HEAPHDR_GET_FLAGS_RAW(hdr));
#if defined(DUK_USE_REFERENCE_COUNTING)
	duk_debug_write_int(thr, (duk_int32_t) DUK_HEAPHDR_GET_REFCOUNT(hdr));
#else
	duk_debug_write_int(thr, (duk_int32_t) -1);
#endif

	switch (DUK_HEAPHDR_GET_TYPE(hdr)) {
	case DUK_HTYPE_STRING: {
		duk_hstring *h = (duk_hstring *) hdr;

		/* FIXME: write_uint? */
		duk_debug_write_int(thr, (duk_int32_t) DUK_HSTRING_GET_BYTELEN(h));
		duk_debug_write_int(thr, (duk_int32_t) DUK_HSTRING_GET_CHARLEN(h));
		duk_debug_write_int(thr, (duk_int32_t) DUK_HSTRING_GET_HASH(h));
		duk_debug_write_hstring(thr, h);
		break;
	}
	case DUK_HTYPE_OBJECT: {
		duk_hobject *h = (duk_hobject *) hdr;
		duk_hstring *k;
		duk_uint_fast32_t i;

		duk_debug_write_int(thr, (duk_int32_t) DUK_HOBJECT_GET_CLASS_NUMBER(h));
		duk_debug_write_heapptr(thr, (duk_heaphdr *) DUK_HOBJECT_GET_PROTOTYPE(heap, h));
		duk_debug_write_int(thr, (duk_int32_t) DUK_HOBJECT_GET_ESIZE(h));
		duk_debug_write_int(thr, (duk_int32_t) DUK_HOBJECT_GET_ENEXT(h));
		duk_debug_write_int(thr, (duk_int32_t) DUK_HOBJECT_GET_ASIZE(h));
		duk_debug_write_int(thr, (duk_int32_t) DUK_HOBJECT_GET_HSIZE(h));

		for (i = 0; i < (duk_uint_fast32_t) DUK_HOBJECT_GET_ENEXT(h); i++) {
			duk_debug_write_int(thr, (duk_int32_t) DUK_HOBJECT_E_GET_FLAGS(heap, h, i));
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

		duk_debug_write_int(thr, (duk_int32_t) DUK_HBUFFER_GET_SIZE(h));
		if (DUK_HBUFFER_HAS_DYNAMIC(h)) {
			duk_debug_write_int(thr, (duk_int32_t) DUK_HBUFFER_DYNAMIC_GET_ALLOC_SIZE((duk_hbuffer_dynamic *) h));
		} else {
			duk_debug_write_int(thr, (duk_int32_t) DUK_HBUFFER_GET_SIZE(h));
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
#error unimplemented
DUK_LOCAL void duk__debug_dump_strtab_chain(duk_hthread *thr, duk_heap *heap) {
	DUK_UNREF(thr);
	DUK_UNREF(heap);

	/* FIXME */
}
#endif  /* DUK_USE_STRTAB_CHAIN */

#if defined(DUK_USE_STRTAB_PROBE)
DUK_LOCAL void duk__debug_dump_strtab_probe(duk_hthread *thr, duk_heap *heap) {
	duk_uint32_t i;
	duk_hstring *h;

	DUK_UNREF(thr);
	DUK_UNREF(heap);

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

	/* FIXME: finish first iteration of format. */

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
		DUK_D(DUK_DPRINT("invalid initial byte, drop connection"));
		goto fail;
	}
	}  /* switch initial byte */

	duk__debug_skip_to_eom(thr);
	return;

 fail:
	DUK__SET_CONN_BROKEN(heap);
	return;
}

DUK_INTERNAL void duk_debug_process_messages(duk_hthread *thr) {
	duk_context *ctx = (duk_context *) thr;
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif

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
		} else if (!thr->heap->dbg_paused) {
			if (!duk_debug_read_peek(thr)) {
				DUK_D(DUK_DPRINT("processing debug message, not paused and peek indicated no data, stop processing"));
				break;
			}
			DUK_D(DUK_DPRINT("processing debug message, not paused and peek indicated there is data, handle it"));
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
	}

	DUK_DD(DUK_DDPRINT("top at exit: %ld", (long) duk_get_top(ctx)));

#if defined(DUK_USE_ASSERTIONS)
	/* Easy to get wrong, so assert for it. */
	DUK_ASSERT(entry_top == duk_get_top(ctx));
#endif
}

/*
 *  Breakpoint management
 */

DUK_INTERNAL duk_small_int_t duk_debug_add_breakpoint(duk_hthread *thr, duk_hstring *filename, duk_uint32_t line) {
	duk_heap *heap;
	duk_breakpoint *b;

	/* FIXME: caller must retrigger recomputation of active breakpoints list.
	 * Would it be safest to write a NULL (empty list) to it here to ensure
	 * no invalid pointers are ever used?
	 */

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(filename != NULL);

	heap = thr->heap;
	if (heap->dbg_breakpoint_count >= DUK_HEAP_MAX_BREAKPOINTS) {
		DUK_D(DUK_DPRINT("failed to add breakpoint for %O:%ld, all breakpoint slots used",
		                 (duk_heaphdr *) filename, (long) line));
		return -1;
	}
	b = heap->dbg_breakpoints + (heap->dbg_breakpoint_count++);

	b->filename = filename;
	b->line = line;
	DUK_HSTRING_INCREF(thr, filename);
	return heap->dbg_breakpoint_count - 1;  /* index */
}

DUK_INTERNAL void duk_debug_remove_breakpoint(duk_hthread *thr, duk_small_uint_t breakpoint_index) {
	duk_heap *heap = thr->heap;
	duk_hstring *h;
	duk_breakpoint *b;
	duk_size_t move_size;

	/* FIXME: caller must retrigger recomputation of active breakpoints list.
	 * Would it be safest to write a NULL (empty list) to it here to ensure
	 * no invalid pointers are ever used?
	 */

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(breakpoint_index >= 0);  /* unsigned */

	if (breakpoint_index >= heap->dbg_breakpoint_count) {
		DUK_D(DUK_DPRINT("invalid breakpoint index: %ld", (long) breakpoint_index));
		return;
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

	DUK_HSTRING_DECREF(thr, h);  /* side effects */

	/* Breakpoint entries above the used area are left as garbage. */
}

#undef DUK__SET_CONN_BROKEN

#else  /* DUK_USE_DEBUGGER_SUPPORT */

/* No debugger support. */

#endif  /* DUK_USE_DEBUGGER_SUPPORT */
