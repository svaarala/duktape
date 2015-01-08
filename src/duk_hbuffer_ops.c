/*
 *  duk_hbuffer operations such as resizing and inserting/appending data to
 *  a dynamic buffer.
 *
 *  Append operations append to the end of the buffer and they are relatively
 *  efficient: the buffer is grown with a "spare" part relative to the buffer
 *  size to minimize reallocations.  Insert operations need to move existing
 *  data forward in the buffer with memmove() and are not very efficient.
 *  They are used e.g. by the regexp compiler to "backpatch" regexp bytecode.
 */

#include "duk_internal.h"

/*
 *  Resizing
 */

DUK_LOCAL duk_size_t duk__add_spare(duk_size_t size) {
	duk_size_t spare = (size / DUK_HBUFFER_SPARE_DIVISOR) + DUK_HBUFFER_SPARE_ADD;
	duk_size_t res;

	res = size + spare;
	if (res < size) {
		/* XXX: handle corner cases where size is close to size limit (wraparound) */
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "duk_size_t wrapped");
	}
	DUK_ASSERT(res >= size);

	return res;
}

DUK_INTERNAL void duk_hbuffer_resize(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t new_size, duk_size_t new_alloc_size) {
	void *res;
	duk_size_t prev_alloc_size;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(new_alloc_size >= new_size);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));

	/*
	 *  Maximum size check
	 */

	if (new_alloc_size > DUK_HBUFFER_MAX_BYTELEN) {
		DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, "buffer too long");
	}

	/*
	 *  Note: use indirect realloc variant just in case mark-and-sweep
	 *  (finalizers) might resize this same buffer during garbage
	 *  collection.
	 */

	res = DUK_REALLOC_INDIRECT(thr->heap, duk_hbuffer_get_dynalloc_ptr, (void *) buf, new_alloc_size);
	if (res != NULL || new_alloc_size == 0) {
		/* 'res' may be NULL if new allocation size is 0. */

		DUK_DDD(DUK_DDDPRINT("resized dynamic buffer %p:%ld:%ld -> %p:%ld:%ld",
		                     (void *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, buf),
		                     (long) DUK_HBUFFER_DYNAMIC_GET_SIZE(buf),
		                     (long) DUK_HBUFFER_DYNAMIC_GET_ALLOC_SIZE(buf),
		                     (void *) res,
		                     (long) new_size,
		                     (long) new_alloc_size));

		/*
		 *  The entire allocated buffer area, regardless of actual used
		 *  size, is kept zeroed in resizes for simplicity.  If the buffer
		 *  is grown, zero the new part.  Another policy would be to
		 *  ensure data is zeroed as the used part is extended.  The
		 *  current approach is much more simple and is not a big deal
		 *  because the spare part is relatively small.
		 */

		prev_alloc_size = DUK_HBUFFER_DYNAMIC_GET_ALLOC_SIZE(buf);
		if (new_alloc_size > prev_alloc_size) {
			DUK_ASSERT(new_alloc_size - prev_alloc_size > 0);
#ifdef DUK_USE_ZERO_BUFFER_DATA
			DUK_MEMZERO((void *) ((char *) res + prev_alloc_size),
			            new_alloc_size - prev_alloc_size);
#endif
		}

		DUK_HBUFFER_DYNAMIC_SET_SIZE(buf, new_size);
		DUK_HBUFFER_DYNAMIC_SET_ALLOC_SIZE(buf, new_alloc_size);
		DUK_HBUFFER_DYNAMIC_SET_DATA_PTR(thr->heap, buf, res);
	} else {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "buffer resize failed: %ld:%ld to %ld:%ld",
		          (long) DUK_HBUFFER_DYNAMIC_GET_SIZE(buf),
		          (long) DUK_HBUFFER_DYNAMIC_GET_ALLOC_SIZE(buf),
		          (long) new_size,
		          (long) new_alloc_size);
	}

	DUK_ASSERT(res != NULL || new_alloc_size == 0);
}

DUK_INTERNAL void duk_hbuffer_reset(duk_hthread *thr, duk_hbuffer_dynamic *buf) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));

	duk_hbuffer_resize(thr, buf, 0, 0);
}

#if 0  /*unused*/
DUK_INTERNAL void duk_hbuffer_compact(duk_hthread *thr, duk_hbuffer_dynamic *buf) {
	duk_size_t curr_size;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));

	curr_size = DUK_HBUFFER_GET_SIZE(buf);
	duk_hbuffer_resize(thr, buf, curr_size, curr_size);
}
#endif

/*
 *  Inserts
 */

DUK_INTERNAL void duk_hbuffer_insert_bytes(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, const duk_uint8_t *data, duk_size_t length) {
	duk_uint8_t *p;

	/* XXX: allow inserts with offset > curr_size? i.e., insert zeroes automatically? */

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));
	DUK_ASSERT_DISABLE(offset >= 0);  /* unsigned, so always true */
	DUK_ASSERT(offset <= DUK_HBUFFER_GET_SIZE(buf));  /* equality is OK (= append) */
	DUK_ASSERT(data != NULL);
	DUK_ASSERT_DISABLE(length >= 0);  /* unsigned, so always true */

	if (length == 0) {
		return;
	}

	if (DUK_HBUFFER_DYNAMIC_GET_SPARE_SIZE(buf) < length) {
		duk_hbuffer_resize(thr,
		                   buf,
		                   DUK_HBUFFER_GET_SIZE(buf),
		                   duk__add_spare(DUK_HBUFFER_GET_SIZE(buf) + length));
	}
	DUK_ASSERT(DUK_HBUFFER_DYNAMIC_GET_SPARE_SIZE(buf) >= length);

	p = (duk_uint8_t *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, buf);
	if (offset < DUK_HBUFFER_GET_SIZE(buf)) {
		/* not an append */

		DUK_ASSERT(DUK_HBUFFER_GET_SIZE(buf) - offset > 0);
		DUK_MEMMOVE((void *) (p + offset + length),
		            (void *) (p + offset),
		            DUK_HBUFFER_GET_SIZE(buf) - offset);
	}

	DUK_ASSERT(length > 0);
	DUK_MEMCPY((void *) (p + offset),
	           data,
	           length);

	DUK_HBUFFER_DYNAMIC_ADD_SIZE(buf, length);
}

#if 0  /*unused*/
DUK_INTERNAL void duk_hbuffer_insert_byte(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, duk_uint8_t byte) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));

	duk_hbuffer_insert_bytes(thr, buf, offset, &byte, 1);
}
#endif

#if 0  /*unused*/
DUK_INTERNAL duk_size_t duk_hbuffer_insert_cstring(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, const char *str) {
	duk_size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(str != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));

	len = DUK_STRLEN(str);
	duk_hbuffer_insert_bytes(thr, buf, offset, (duk_uint8_t *) str, len);
	return len;
}
#endif

#if 0  /*unused*/
DUK_INTERNAL duk_size_t duk_hbuffer_insert_hstring(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, duk_hstring *str) {
	duk_size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(str != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));

	len = DUK_HSTRING_GET_BYTELEN(str);
	duk_hbuffer_insert_bytes(thr, buf, offset, (duk_uint8_t *) DUK_HSTRING_GET_DATA(str), len);
	return len;
}
#endif

DUK_INTERNAL duk_size_t duk_hbuffer_insert_xutf8(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, duk_ucodepoint_t codepoint) {
	duk_uint8_t tmp[DUK_UNICODE_MAX_XUTF8_LENGTH];
	duk_size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));
	/* No range assertion for 'codepoint' */

	/* Intentionally no fast path: insertion is not that central */

	len = (duk_size_t) duk_unicode_encode_xutf8(codepoint, tmp);
	duk_hbuffer_insert_bytes(thr, buf, offset, tmp, len);
	return len;
}

/* Append a Unicode codepoint to the buffer in CESU-8 format, i.e., convert
 * non-BMP characters to surrogate pairs which are then "UTF-8" encoded.
 * If the codepoint is initially a surrogate, it is also encoded into CESU-8.
 * Codepoints above valid Unicode range (> U+10FFFF) are mangled.
 */

#if 0  /*unused*/
DUK_INTERNAL duk_size_t duk_hbuffer_insert_cesu8(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, duk_ucodepoint_t codepoint) {
	duk_uint8_t tmp[DUK_UNICODE_MAX_CESU8_LENGTH];
	duk_size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));
	DUK_ASSERT_DISABLE(codepoint >= 0);  /* unsigned */
	DUK_ASSERT(codepoint <= 0x10ffff);  /* if not in this range, results are garbage (but no crash) */

	/* Intentionally no fast path: insertion is not that central */

	len = (duk_size_t) duk_unicode_encode_cesu8(codepoint, tmp);
	duk_hbuffer_insert_bytes(thr, buf, offset, tmp, len);
	return len;
}
#endif

/*
 *  Appends
 *
 *  Note: an optimized duk_hbuffer_append_bytes() could be implemented, but
 *  it is more compact to use duk_hbuffer_insert_bytes() instead.  The
 *  important fast paths bypass these functions. anyway.
 */

DUK_INTERNAL void duk_hbuffer_append_bytes(duk_hthread *thr, duk_hbuffer_dynamic *buf, const duk_uint8_t *data, duk_size_t length) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));
	DUK_ASSERT(data != NULL);

	duk_hbuffer_insert_bytes(thr, buf, DUK_HBUFFER_GET_SIZE(buf), data, length);
}

DUK_INTERNAL void duk_hbuffer_append_byte(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_uint8_t byte) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));

	duk_hbuffer_insert_bytes(thr, buf, DUK_HBUFFER_GET_SIZE(buf), &byte, 1);
}

DUK_INTERNAL duk_size_t duk_hbuffer_append_cstring(duk_hthread *thr, duk_hbuffer_dynamic *buf, const char *str) {
	duk_size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(str != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));

	len = DUK_STRLEN(str);
	duk_hbuffer_insert_bytes(thr, buf, DUK_HBUFFER_GET_SIZE(buf), (duk_uint8_t *) str, len);
	return len;
}

DUK_INTERNAL duk_size_t duk_hbuffer_append_hstring(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_hstring *str) {
	duk_size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(str != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));

	len = DUK_HSTRING_GET_BYTELEN(str);
	duk_hbuffer_insert_bytes(thr, buf, DUK_HBUFFER_GET_SIZE(buf), (duk_uint8_t *) DUK_HSTRING_GET_DATA(str), len);
	return len;
}

/* Append a Unicode codepoint to the buffer in extended UTF-8 format, i.e.
 * allow codepoints above standard Unicode range (> U+10FFFF) up to seven
 * byte encoding (36 bits, but argument type is 32 bits).  In particular,
 * allows encoding of all unsigned 32-bit integers.  If the codepoint is
 * initially a surrogate, it is encoded without checking (and will become,
 * effectively, CESU-8 encoded).
 */

DUK_INTERNAL duk_size_t duk_hbuffer_append_xutf8(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_ucodepoint_t codepoint) {
	duk_uint8_t tmp[DUK_UNICODE_MAX_XUTF8_LENGTH];
	duk_size_t len;
	duk_size_t sz;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));
	/* No range assertion for 'codepoint' */

	if (DUK_LIKELY(codepoint < 0x80 && DUK_HBUFFER_DYNAMIC_GET_SPARE_SIZE(buf) > 0)) {
		/* fast path: ASCII and there is spare */
		duk_uint8_t *p = ((duk_uint8_t *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, buf));
		sz = DUK_HBUFFER_DYNAMIC_GET_SIZE(buf);
		p[sz++] = (duk_uint8_t) codepoint;
		DUK_HBUFFER_DYNAMIC_SET_SIZE(buf, sz);
		return 1;
	}

	len = (duk_size_t) duk_unicode_encode_xutf8(codepoint, tmp);
	duk_hbuffer_insert_bytes(thr, buf, DUK_HBUFFER_GET_SIZE(buf), tmp, len);
	return len;
}

/* Append a Unicode codepoint to the buffer in CESU-8 format, i.e., convert
 * non-BMP characters to surrogate pairs which are then "UTF-8" encoded.
 * If the codepoint is initially a surrogate, it is also encoded into CESU-8.
 * Codepoints above valid Unicode range (> U+10FFFF) are mangled.
 */

DUK_INTERNAL duk_size_t duk_hbuffer_append_cesu8(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_ucodepoint_t codepoint) {
	duk_uint8_t tmp[DUK_UNICODE_MAX_CESU8_LENGTH];
	duk_size_t len;
	duk_size_t sz;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));
	DUK_ASSERT_DISABLE(codepoint >= 0);  /* unsigned */
	DUK_ASSERT(codepoint <= 0x10ffff);  /* if not in this range, results are garbage (but no crash) */

	if (DUK_LIKELY(codepoint < 0x80 && DUK_HBUFFER_DYNAMIC_GET_SPARE_SIZE(buf) > 0)) {
		/* fast path: ASCII and there is spare */
		duk_uint8_t *p = ((duk_uint8_t *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, buf));
		sz = DUK_HBUFFER_DYNAMIC_GET_SIZE(buf);
		p[sz++] = (duk_uint8_t) codepoint;
		DUK_HBUFFER_DYNAMIC_SET_SIZE(buf, sz);
		return 1;
	}

	len = (duk_size_t) duk_unicode_encode_cesu8(codepoint, tmp);
	duk_hbuffer_insert_bytes(thr, buf, DUK_HBUFFER_GET_SIZE(buf), tmp, len);
	return len;
}

/* Append an duk_uint32_t in native byte order. */
#if 0  /*unused*/
DUK_INTERNAL void duk_hbuffer_append_native_u32(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_uint32_t val) {
	/* relies on duk_uint32_t being exactly right size */
	DUK_ASSERT(sizeof(val) == 4);
	duk_hbuffer_insert_bytes(thr,
	                         buf,
	                         DUK_HBUFFER_GET_SIZE(buf),
	                         (duk_uint8_t *) &val,
	                         sizeof(duk_uint32_t));
}
#endif

/*
 *  In-buffer "slices"
 *
 *  Slices are identified with an offset+length pair, referring to the current
 *  buffer data.  A caller cannot otherwise reliably refer to existing data,
 *  because the buffer may be reallocated before a data pointer is referenced.
 */

DUK_INTERNAL void duk_hbuffer_remove_slice(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t offset, duk_size_t length) {
	duk_uint8_t *p;
	duk_size_t end_offset;

	DUK_UNREF(thr);

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));
	DUK_ASSERT_DISABLE(offset >= 0);                               /* always true */
	DUK_ASSERT(offset <= DUK_HBUFFER_GET_SIZE(buf));               /* allow equality */
	DUK_ASSERT_DISABLE(length >= 0);                               /* always true */
	DUK_ASSERT(offset + length <= DUK_HBUFFER_GET_SIZE(buf));      /* allow equality */

	if (length == 0) {
		return;
	}

	p = (duk_uint8_t *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, buf);

	end_offset = offset + length;

	if (end_offset < DUK_HBUFFER_GET_SIZE(buf)) {
		/* not strictly from end of buffer; need to shuffle data */
		DUK_ASSERT(DUK_HBUFFER_GET_SIZE(buf) - end_offset > 0);
		DUK_MEMMOVE(p + offset,
		            p + end_offset,
		            DUK_HBUFFER_GET_SIZE(buf) - end_offset);
	}

	/* Here we want to zero data even with automatic buffer zeroing
	 * disabled as we depend on this internally too.
	 */
	DUK_ASSERT(length > 0);
	DUK_MEMZERO(p + DUK_HBUFFER_GET_SIZE(buf) - length,
	            length);

	DUK_HBUFFER_DYNAMIC_SUB_SIZE(buf, length);

	/* Note: no shrink check, intentional */
}

DUK_INTERNAL void duk_hbuffer_insert_slice(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t dst_offset, duk_size_t src_offset, duk_size_t length) {
	duk_uint8_t *p;
	duk_size_t src_end_offset;  /* source end (exclusive) in initial buffer */
	duk_size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));
	DUK_ASSERT_DISABLE(dst_offset >= 0);                           /* always true */
	DUK_ASSERT(dst_offset <= DUK_HBUFFER_GET_SIZE(buf));           /* allow equality */
	DUK_ASSERT_DISABLE(src_offset >= 0);                           /* always true */
	DUK_ASSERT(src_offset <= DUK_HBUFFER_GET_SIZE(buf));           /* allow equality */
	DUK_ASSERT_DISABLE(length >= 0);                               /* always true */
	DUK_ASSERT(src_offset + length <= DUK_HBUFFER_GET_SIZE(buf));  /* allow equality */

	if (length == 0) {
		return;
	}

	if (DUK_HBUFFER_DYNAMIC_GET_SPARE_SIZE(buf) < length) {
		duk_hbuffer_resize(thr,
		                   buf,
		                   DUK_HBUFFER_GET_SIZE(buf),
		                   duk__add_spare(DUK_HBUFFER_GET_SIZE(buf) + length));
	}
	DUK_ASSERT(DUK_HBUFFER_DYNAMIC_GET_SPARE_SIZE(buf) >= length);

	p = (duk_uint8_t *) DUK_HBUFFER_DYNAMIC_GET_DATA_PTR(thr->heap, buf);
	DUK_ASSERT(p != NULL);  /* must be the case because length > 0, and buffer has been resized if necessary */

	/*
	 *  src_offset and dst_offset refer to the state of the buffer
	 *  before any changes are made.  This must be taken into account
	 *  when moving data around; in particular, the source data may
	 *  "straddle" the dst_offset, so the insert may need to be handled
	 *  in two pieces.
	 */

	src_end_offset = src_offset + length;

	/* create a hole for the insert */
	len = DUK_HBUFFER_GET_SIZE(buf) - dst_offset;
	DUK_MEMMOVE(p + dst_offset + length,
	            p + dst_offset,
	            len);  /* zero size is not an issue: pointers are valid */

	if (src_offset < dst_offset) {
		if (src_end_offset <= dst_offset) {
			/* entire source is before 'dst_offset' */
			DUK_MEMCPY(p + dst_offset,
			           p + src_offset,
			           length);
		} else {
			/* part of the source is before 'dst_offset'; straddles */
			len = dst_offset - src_offset;
			DUK_ASSERT(len >= 1 && len < length);
			DUK_ASSERT(length - len >= 1);
			DUK_MEMCPY(p + dst_offset,
			           p + src_offset,
			           len);
			DUK_MEMCPY(p + dst_offset + len,
			           p + src_offset + length + len,  /* take above memmove() into account */
			           length - len);
		}
	} else {
		/* entire source is after 'dst_offset' */
		DUK_MEMCPY(p + dst_offset,
		           p + src_offset + length,  /* take above memmove() into account */
		           length);
	}

	DUK_HBUFFER_DYNAMIC_ADD_SIZE(buf, length);
}

DUK_INTERNAL void duk_hbuffer_append_slice(duk_hthread *thr, duk_hbuffer_dynamic *buf, duk_size_t src_offset, duk_size_t length) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(buf));
	DUK_ASSERT_DISABLE(src_offset >= 0);                           /* always true */
	DUK_ASSERT(src_offset <= DUK_HBUFFER_GET_SIZE(buf));           /* allow equality */
	DUK_ASSERT_DISABLE(length >= 0);                               /* always true */
	DUK_ASSERT(src_offset + length <= DUK_HBUFFER_GET_SIZE(buf));  /* allow equality */

	duk_hbuffer_insert_slice(thr,
	                         buf,
	                         DUK_HBUFFER_GET_SIZE(buf),
	                         src_offset,
	                         length);
}
