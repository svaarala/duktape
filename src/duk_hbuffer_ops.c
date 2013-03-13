/*
 *  duk_hbuffer operations such as resizing and inserting/appending data to
 *  a growable buffer.
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

static size_t add_spare(size_t size) {
	size_t spare = (size / DUK_HBUFFER_SPARE_DIVISOR) + DUK_HBUFFER_SPARE_ADD;
	size_t res;

	/* FIXME: handle corner cases where size is close to size limit (wraparound) */
	res = size + spare;
	DUK_ASSERT(res >= size);

	return res;
}

void duk_hbuffer_resize(duk_hthread *thr, duk_hbuffer_growable *buf, size_t new_size, size_t new_usable_size) {
	size_t alloc_size;
	void **ptr;
	void *res;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(new_usable_size >= new_size);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));

	/*
	 *  Note: use indirect realloc variant just in case mark-and-sweep
	 *  (finalizers) might resize this same buffer during garbage
	 *  collection.
	 */

	/* FIXME: maybe remove safety NUL term for buffers? */
	alloc_size = new_usable_size + 1;  /* +1 for safety nul term */
	ptr = &buf->curr_alloc;
	res = DUK_REALLOC_INDIRECT(thr->heap, ptr, alloc_size);
	if (res) {
		DUK_DDDPRINT("resized growable buffer %p:%d:%d -> %p:%d:%d",
		             buf->curr_alloc, buf->size, buf->usable_size,
		             res, new_size, new_usable_size);

		/*
		 *  All data in [new_size,new_alloc_size[ should be zeroed.
		 *
		 *  The current memset could be optimized by taking advantage of
		 *  the knowledge that the old "spare" area is already zeroed
		 *  (an invariant which is maintained at all times).  So, this
		 *  could be optimized to two smaller memsets, one to handle a
		 *  size change, and another to handle an alloc size change.
		 *  However, this would probably not be useful in practice, as
		 *  the spare is usually very small.
		 */

		DUK_ASSERT(new_usable_size + 1 > new_size);
		memset((void *) ((char *) res + new_size),
		       0,
		       new_usable_size + 1 - new_size);

		buf->size = new_size;
		buf->usable_size = new_usable_size;
		*ptr = res;
	} else {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to resize growable buffer from %d:%d to %d:%d",
		          buf->size, buf->usable_size, new_size, new_usable_size);
	}

	DUK_ASSERT(res != NULL);
}

void duk_hbuffer_reset(duk_hthread *thr, duk_hbuffer_growable *buf) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));

	duk_hbuffer_resize(thr, buf, 0, 0);
}

void duk_hbuffer_compact(duk_hthread *thr, duk_hbuffer_growable *buf) {
	size_t curr_size;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));

	curr_size = DUK_HBUFFER_GET_SIZE(buf);
	duk_hbuffer_resize(thr, buf, curr_size, curr_size);
}

/*
 *  Inserts
 */

void duk_hbuffer_insert_bytes(duk_hthread *thr, duk_hbuffer_growable *buf, size_t offset, duk_u8 *data, size_t length) {
	char *p;

	/* XXX: allow inserts with offset > curr_size? i.e., insert zeroes automatically? */

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));
	DUK_ASSERT(offset >= 0);  /* unsigned, so always true */
	DUK_ASSERT(offset <= DUK_HBUFFER_GET_SIZE(buf));  /* equality is OK (= append) */
	DUK_ASSERT(data != NULL);
	DUK_ASSERT(length >= 0);  /* unsigned, so always true */

	if (length == 0) {
		return;
	}

	if (DUK_HBUFFER_GROWABLE_GET_SPARE_SIZE(buf) < length) {
		duk_hbuffer_resize(thr,
		                   buf,
		                   DUK_HBUFFER_GET_SIZE(buf),
		                   add_spare(DUK_HBUFFER_GET_SIZE(buf) + length));
	}
	DUK_ASSERT(DUK_HBUFFER_GROWABLE_GET_SPARE_SIZE(buf) >= length);

	p = (char *) DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(buf);
	if (offset < DUK_HBUFFER_GET_SIZE(buf)) {
		/* not an append */

		DUK_ASSERT(DUK_HBUFFER_GET_SIZE(buf) - offset > 0);  /* not a zero byte memmove */
		memmove((void *) (p + offset + length),
		        (void *) (p + offset),
		        DUK_HBUFFER_GET_SIZE(buf) - offset);
	}

	memcpy((void *) (p + offset),
	       data,
	       length);

	buf->size += length;
}

void duk_hbuffer_insert_byte(duk_hthread *thr, duk_hbuffer_growable *buf, size_t offset, duk_u8 byte) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));

	duk_hbuffer_insert_bytes(thr, buf, offset, &byte, 1);
}

size_t duk_hbuffer_insert_cstring(duk_hthread *thr, duk_hbuffer_growable *buf, size_t offset, const char *str) {
	size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(str != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));

	len = strlen(str);
	duk_hbuffer_insert_bytes(thr, buf, offset, (duk_u8 *) str, len);
	return len;
}

size_t duk_hbuffer_insert_hstring(duk_hthread *thr, duk_hbuffer_growable *buf, size_t offset, duk_hstring *str) {
	size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(str != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));

	len = DUK_HSTRING_GET_BYTELEN(str);
	duk_hbuffer_insert_bytes(thr, buf, offset, (duk_u8 *) DUK_HSTRING_GET_DATA(str), len);
	return len;
}

size_t duk_hbuffer_insert_xutf8(duk_hthread *thr, duk_hbuffer_growable *buf, size_t offset, duk_u32 codepoint) {
	duk_u8 tmp[DUK_UNICODE_MAX_XUTF8_LENGTH];
	size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));
	/* No range assertion for 'codepoint' */

	/* Intentionally no fast path: insertion is not that central */

	len = duk_unicode_encode_xutf8(codepoint, tmp);
	duk_hbuffer_insert_bytes(thr, buf, offset, tmp, len);
	return len;
}

/* Append a Unicode codepoint to the buffer in CESU-8 format, i.e., convert
 * non-BMP characters to surrogate pairs which are then "UTF-8" encoded.
 * If the codepoint is initially a surrogate, it is also encoded into CESU-8.
 * Codepoints above valid Unicode range (> U+10FFFF) are mangled.
 */

size_t duk_hbuffer_insert_cesu8(duk_hthread *thr, duk_hbuffer_growable *buf, size_t offset, duk_u32 codepoint) {
	duk_u8 tmp[DUK_UNICODE_MAX_CESU8_LENGTH];
	size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));
	DUK_ASSERT(codepoint >= 0 && codepoint <= 0x10ffff);  /* if not in this range, results are garbage (but no crash) */

	/* Intentionally no fast path: insertion is not that central */

	len = duk_unicode_encode_cesu8(codepoint, tmp);
	duk_hbuffer_insert_bytes(thr, buf, offset, tmp, len);
	return len;
}

/*
 *  Appends
 *
 *  Note: an optimized duk_hbuffer_append_bytes() could be implemented, but
 *  it is more compact to use duk_hbuffer_insert_bytes() instead.  The
 *  important fast paths bypass these functions. anyway.
 */

void duk_hbuffer_append_bytes(duk_hthread *thr, duk_hbuffer_growable *buf, duk_u8 *data, size_t length) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));
	DUK_ASSERT(data != NULL);

	duk_hbuffer_insert_bytes(thr, buf, DUK_HBUFFER_GET_SIZE(buf), data, length);
}

void duk_hbuffer_append_byte(duk_hthread *thr, duk_hbuffer_growable *buf, duk_u8 byte) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));

	duk_hbuffer_insert_bytes(thr, buf, DUK_HBUFFER_GET_SIZE(buf), &byte, 1);
}

size_t duk_hbuffer_append_cstring(duk_hthread *thr, duk_hbuffer_growable *buf, const char *str) {
	size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(str != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));

	len = strlen(str);
	duk_hbuffer_insert_bytes(thr, buf, DUK_HBUFFER_GET_SIZE(buf), (duk_u8 *) str, len);
	return len;
}

size_t duk_hbuffer_append_hstring(duk_hthread *thr, duk_hbuffer_growable *buf, duk_hstring *str) {
	size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(str != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));

	len = DUK_HSTRING_GET_BYTELEN(str);
	duk_hbuffer_insert_bytes(thr, buf, DUK_HBUFFER_GET_SIZE(buf), (duk_u8 *) DUK_HSTRING_GET_DATA(str), len);
	return len;
}

/* Append a Unicode codepoint to the buffer in extended UTF-8 format, i.e.
 * allow codepoints above standard Unicode range (> U+10FFFF) up to seven
 * byte encoding (36 bits, but argument type is 32 bits).  In particular,
 * allows encoding of all unsigned 32-bit integers.  If the codepoint is
 * initially a surrogate, it is encoded without checking (and will become,
 * effectively, CESU-8 encoded).
 */

size_t duk_hbuffer_append_xutf8(duk_hthread *thr, duk_hbuffer_growable *buf, duk_u32 codepoint) {
	duk_u8 tmp[DUK_UNICODE_MAX_XUTF8_LENGTH];
	size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));
	/* No range assertion for 'codepoint' */

	if (codepoint < 0x80 && DUK_HBUFFER_GROWABLE_GET_SPARE_SIZE(buf) > 0) {
		/* fast path: ASCII and there is spare */
		duk_u8 *p = ((duk_u8 *) DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(buf)) + DUK_HBUFFER_GET_SIZE(buf);
		*p = (duk_u8) codepoint;
		buf->size += 1;
		return 1;
	}

	len = duk_unicode_encode_xutf8(codepoint, tmp);
	duk_hbuffer_insert_bytes(thr, buf, DUK_HBUFFER_GET_SIZE(buf), tmp, len);
	return len;
}

/* Append a Unicode codepoint to the buffer in CESU-8 format, i.e., convert
 * non-BMP characters to surrogate pairs which are then "UTF-8" encoded.
 * If the codepoint is initially a surrogate, it is also encoded into CESU-8.
 * Codepoints above valid Unicode range (> U+10FFFF) are mangled.
 */

size_t duk_hbuffer_append_cesu8(duk_hthread *thr, duk_hbuffer_growable *buf, duk_u32 codepoint) {
	duk_u8 tmp[DUK_UNICODE_MAX_CESU8_LENGTH];
	size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));
	DUK_ASSERT(codepoint >= 0 && codepoint <= 0x10ffff);  /* if not in this range, results are garbage (but no crash) */

	if (codepoint < 0x80 && DUK_HBUFFER_GROWABLE_GET_SPARE_SIZE(buf) > 0) {
		/* fast path: ASCII and there is spare */
		duk_u8 *p = ((duk_u8 *) DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(buf)) + DUK_HBUFFER_GET_SIZE(buf);
		*p = (duk_u8) codepoint;
		buf->size += 1;
		return 1;
	}

	len = duk_unicode_encode_cesu8(codepoint, tmp);
	duk_hbuffer_insert_bytes(thr, buf, DUK_HBUFFER_GET_SIZE(buf), tmp, len);
	return len;
}

/* Append an duk_u32 in native byte order.  This is used to emit bytecode
 * instructions.
 */

void duk_hbuffer_append_native_u32(duk_hthread *thr, duk_hbuffer_growable *buf, duk_u32 val) {
	/* FIXME: relies on duk_u32 being exactly right size */
	duk_hbuffer_insert_bytes(thr,
	                         buf,
	                         DUK_HBUFFER_GET_SIZE(buf),
	                         (duk_u8 *) &val,
	                         sizeof(duk_u32));
}

/*
 *  In-buffer "slices"
 *
 *  Slices are identified with an offset+length pair, referring to the current
 *  buffer data.  A caller cannot otherwise reliably refer to existing data,
 *  because the buffer may be reallocated before a data pointer is referenced.
 */

void duk_hbuffer_remove_slice(duk_hthread *thr, duk_hbuffer_growable *buf, size_t offset, size_t length) {
	char *p;
	size_t end_offset;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));
	DUK_ASSERT(offset >= 0);                                       /* always true */
	DUK_ASSERT(offset <= DUK_HBUFFER_GET_SIZE(buf));               /* allow equality */
	DUK_ASSERT(length >= 0);                                       /* always true */
	DUK_ASSERT(offset + length <= DUK_HBUFFER_GET_SIZE(buf));      /* allow equality */

	if (length == 0) {
		return;
	}

	p = (char *) DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(buf);

	end_offset = offset + length;

	if (end_offset < DUK_HBUFFER_GET_SIZE(buf)) {
		/* not strictly from end of buffer; need to shuffle data */
		memmove(p + offset,
		        p + end_offset,
	                DUK_HBUFFER_GET_SIZE(buf) - end_offset);  /* always > 0 */
	}

	memset(p + DUK_HBUFFER_GET_SIZE(buf) - length,
	       0,
	       length);  /* always > 0 */

	buf->size -= length;

	/* Note: no shrink check, intentional */
}

void duk_hbuffer_insert_slice(duk_hthread *thr, duk_hbuffer_growable *buf, size_t dst_offset, size_t src_offset, size_t length) {
	char *p;
	size_t src_end_offset;  /* source end (exclusive) in initial buffer */
	size_t len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));
	DUK_ASSERT(dst_offset >= 0);                                   /* always true */
	DUK_ASSERT(dst_offset <= DUK_HBUFFER_GET_SIZE(buf));           /* allow equality */
	DUK_ASSERT(src_offset >= 0);                                   /* always true */
	DUK_ASSERT(src_offset <= DUK_HBUFFER_GET_SIZE(buf));           /* allow equality */
	DUK_ASSERT(length >= 0);                                       /* always true */
	DUK_ASSERT(src_offset + length <= DUK_HBUFFER_GET_SIZE(buf));  /* allow equality */

	if (length == 0) {
		return;
	}

	if (DUK_HBUFFER_GROWABLE_GET_SPARE_SIZE(buf) < length) {
		duk_hbuffer_resize(thr,
		                   buf,
		                   DUK_HBUFFER_GET_SIZE(buf),
		                   add_spare(DUK_HBUFFER_GET_SIZE(buf) + length));
	}
	DUK_ASSERT(DUK_HBUFFER_GROWABLE_GET_SPARE_SIZE(buf) >= length);

	p = (char *) DUK_HBUFFER_GROWABLE_GET_CURR_DATA_PTR(buf);

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
	if (len > 0) {
		memmove(p + dst_offset + length,
		        p + dst_offset,
		        len);
	}

	if (src_offset < dst_offset) {
		if (src_end_offset <= dst_offset) {
			/* entire source is before 'dst_offset' */
			memcpy(p + dst_offset,
			       p + src_offset,
			       length);
		} else {
			/* part of the source is before 'dst_offset'; straddles */
			len = dst_offset - src_offset;
			DUK_ASSERT(len >= 1 && len < length);
			DUK_ASSERT(length - len >= 1);
			memcpy(p + dst_offset,
			       p + src_offset,
			       len);
			memcpy(p + dst_offset + len,
			       p + src_offset + length + len,  /* take above memmove() into account */
			       length - len);
		}
	} else {
		/* entire source is after 'dst_offset' */
		memcpy(p + dst_offset,
		       p + src_offset + length,  /* take above memmove() into account */
		       length);
	}

	buf->size += length;
}

void duk_hbuffer_append_slice(duk_hthread *thr, duk_hbuffer_growable *buf, size_t src_offset, size_t length) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(buf));
	DUK_ASSERT(src_offset >= 0);                                   /* always true */
	DUK_ASSERT(src_offset <= DUK_HBUFFER_GET_SIZE(buf));           /* allow equality */
	DUK_ASSERT(length >= 0);                                       /* always true */
	DUK_ASSERT(src_offset + length <= DUK_HBUFFER_GET_SIZE(buf));  /* allow equality */

	duk_hbuffer_insert_slice(thr,
	                         buf,
	                         DUK_HBUFFER_GET_SIZE(buf),
	                         src_offset,
	                         length);
}

