/*
 *  String hash computation (interning).
 */

#include "duk_internal.h"

#if defined(DUK_USE_STRHASH_DENSE)
/* constants for duk_hashstring() */
#define DUK__STRHASH_SHORTSTRING   4096L
#define DUK__STRHASH_MEDIUMSTRING  (256L * 1024L)
#define DUK__STRHASH_BLOCKSIZE     256L

DUK_INTERNAL duk_uint32_t duk_heap_hashstring(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;

	/*
	 *  Sampling long strings by byte skipping (like Lua does) is potentially
	 *  a cache problem.  Here we do 'block skipping' instead for long strings:
	 *  hash an initial part, and then sample the rest of the string with
	 *  reasonably sized chunks.
	 *
	 *  Skip should depend on length and bound the total time to roughly
	 *  logarithmic.
	 *
	 *  With current values:
	 *
	 *    1M string => 256 * 241 = 61696 bytes (0.06M) of hashing
	 *    1G string => 256 * 16321 = 4178176 bytes (3.98M) of hashing
	 *
	 *  After an initial part has been hashed, an offset is applied before
	 *  starting the sampling.  The initial offset is computed from the
	 *  hash of the initial part of the string.  The idea is to avoid the
	 *  case that all long strings have certain offset ranges that are never
	 *  sampled.
	 */

	/* note: mixing len into seed improves hashing when skipping */
	duk_uint32_t str_seed = heap->hash_seed ^ ((duk_uint32_t) len);

	if (len <= DUK__STRHASH_SHORTSTRING) {
		hash = duk_util_hashbytes(str, len, str_seed);
	} else {
		duk_size_t off;
		duk_size_t skip;

		if (len <= DUK__STRHASH_MEDIUMSTRING) {
			skip = (duk_size_t) (16 * DUK__STRHASH_BLOCKSIZE + DUK__STRHASH_BLOCKSIZE);
		} else {
			skip = (duk_size_t) (256 * DUK__STRHASH_BLOCKSIZE + DUK__STRHASH_BLOCKSIZE);
		}

		hash = duk_util_hashbytes(str, (duk_size_t) DUK__STRHASH_SHORTSTRING, str_seed);
		off = DUK__STRHASH_SHORTSTRING + (skip * (hash % 256)) / 256;

		/* XXX: inefficient loop */
		while (off < len) {
			duk_size_t left = len - off;
			duk_size_t now = (duk_size_t) (left > DUK__STRHASH_BLOCKSIZE ? DUK__STRHASH_BLOCKSIZE : left);
			hash ^= duk_util_hashbytes(str + off, now, str_seed);
			off += skip;
		}
	}

#if defined(DUK_USE_STRHASH16)
	/* Truncate to 16 bits here, so that a computed hash can be compared
	 * against a hash stored in a 16-bit field.
	 */
	hash &= 0x0000ffffUL;
#endif
	return hash;
}
#else  /* DUK_USE_STRHASH_DENSE */
#if 1
DUK_LOCAL duk_uint32_t duk__hashstring_lua1(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;
	duk_size_t step;
	duk_size_t off;

	/* String algorithm based on Lua 5.1.5 with small modifications.
	 * See lstring.c:luaS_newlstr().
	 *
	 * This is basically "Shift-add-XOR hash" with skipping and reverse
	 * direction:
	 * http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
	 */

	hash = heap->hash_seed ^ ((duk_uint32_t) len);
	step = (len >> DUK_USE_STRHASH_SKIP_SHIFT) + 1;
	for (off = len; off >= step; off -= step) {
		DUK_ASSERT(off >= 1);  /* off >= step, and step >= 1 */
		hash = hash ^ ((hash << 5) + (hash >> 2) + str[off - 1]);
	}

	return hash;
}
#endif

#if 1
DUK_LOCAL duk_uint32_t duk__hashstring_lua2(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;
	duk_size_t step;
	duk_size_t off;

	/* Forward stepping variant of Lua 5.1.5. */
	hash = heap->hash_seed ^ ((duk_uint32_t) len);
	step = (len >> DUK_USE_STRHASH_SKIP_SHIFT) + 1;
	off = (len + step - 1) % step;
	for (; off < len; off += step) {
		hash = hash ^ ((hash << 5) + (hash >> 2) + str[off]);
	}

	return hash;
}
#endif

#if 1
DUK_LOCAL duk_uint32_t duk__hashstring_lua3(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;
	duk_size_t step;
	const duk_uint8_t *p;
	const duk_uint8_t *p_stop;

	/* Forward stepping variant of Lua 5.1.5 using pointers. */
	hash = heap->hash_seed ^ ((duk_uint32_t) len);
	if (len > 0) {
		step = (len >> DUK_USE_STRHASH_SKIP_SHIFT) + 1;
		p = str + ((len - 1) % step);
		p_stop = str + len - 1;
		DUK_ASSERT(((duk_size_t) (p_stop - p) % step) == 0);  /* p eventually hits p_stop */
		while (p != p_stop) {
			hash = hash ^ ((hash << 5) + (hash >> 2) + *p);
			p += step;
		}
	}

	return hash;
}
#endif

#if 1
DUK_LOCAL duk_uint32_t duk__hashstring_hybrid1(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;
	duk_size_t step;
	duk_size_t off;

	/* Hybrid with a different algorithm for short and long strings. */
	hash = heap->hash_seed ^ ((duk_uint32_t) len);
	if (DUK_LIKELY(len <= 32)) {
		for (off = 0; off < len; off++) {
			hash = hash ^ ((hash << 5) + (hash >> 2) + str[off]);
		}
	} else {
		step = (len >> DUK_USE_STRHASH_SKIP_SHIFT) + 1;
		for (off = 0; off < len; off += step) {
			hash = hash ^ ((hash << 5) + (hash >> 2) + str[off]);
		}
		DUK_ASSERT(len >= 1);
		hash = hash ^ ((hash << 5) + (hash >> 2) + str[len - 1]);
	}

	return hash;
}
#endif

#if 1
DUK_LOCAL duk_uint32_t duk__hashstring_hybrid2(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;
	duk_size_t step;
	duk_size_t off;
	duk_size_t limit;

	/* Hybrid with a different algorithm for short and long strings.
	 * For long strings, include first and last 8 bytes entirely, and
	 * use sparse skipping for the middle.
	 */
	hash = heap->hash_seed ^ ((duk_uint32_t) len);
	if (DUK_LIKELY(len <= 32)) {
		for (off = 0; off < len; off++) {
			hash = hash ^ ((hash << 5) + (hash >> 2) + str[off]);
		}
	} else {
		for (off = 0; off < 8; off++) {
			hash = hash ^ ((hash << 5) + (hash >> 2) + str[off]);
		}
		for (off = len - 8; off < len; off++) {
			hash = hash ^ ((hash << 5) + (hash >> 2) + str[off]);
		}
		step = (len >> 4);
		limit = len - 8;
		off = 8 + (hash & 0x07);  /* vary offset a bit */
		for (; off < limit; off += step) {
			hash = hash ^ ((hash << 5) + (hash >> 2) + str[off]);
		}
		hash = hash ^ ((hash << 5) + (hash >> 2) + str[len - 1]);
	}

	return hash;
}
#endif

#if 1
DUK_LOCAL duk_uint32_t duk__hashstring_bernstein1a(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;
	duk_size_t step;
	duk_size_t off;

	/* "Bernstein hash" from:
	 * http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
	 * but with string skipping and reverse direction (ensures
	 * last byte is included).
	 */
	hash = heap->hash_seed;
	step = (len >> DUK_USE_STRHASH_SKIP_SHIFT) + 1;
	for (off = len; off >= step; off -= step) {
		DUK_ASSERT(off >= 1);  /* off >= step, and step >= 1 */
		hash = (hash * 33) + str[off - 1];
	}

	return hash;
}
#endif

#if 1
DUK_LOCAL duk_uint32_t duk__hashstring_bernstein1b(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;
	duk_size_t step;
	duk_size_t off;

	/* "Bernstein hash" from:
	 * http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
	 * but with string skipping and reverse direction (ensures
	 * last byte is included).
	 */
	hash = heap->hash_seed;
	step = (len >> DUK_USE_STRHASH_SKIP_SHIFT) + 1;
	for (off = len; off >= step; off -= step) {
		DUK_ASSERT(off >= 1);  /* off >= step, and step >= 1 */
		hash = ((hash << 5) + hash) + str[off - 1];
	}

	return hash;
}
#endif

#if 1
DUK_LOCAL duk_uint32_t duk__hashstring_bernstein2a(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;
	duk_size_t step;
	duk_size_t off;

	/* "Modified Bernstein" from:
	 * http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
	 * but with string skipping and reverse direction (ensures
	 * last byte is included).
	 */
	hash = heap->hash_seed;
	step = (len >> DUK_USE_STRHASH_SKIP_SHIFT) + 1;
	for (off = len; off >= step; off -= step) {
		DUK_ASSERT(off >= 1);  /* off >= step, and step >= 1 */
		hash = (hash * 33) ^ str[off - 1];
	}

	return hash;
}
#endif

#if 1
DUK_LOCAL duk_uint32_t duk__hashstring_bernstein2b(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;
	duk_size_t step;
	duk_size_t off;

	/* "Modified Bernstein" from:
	 * http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
	 * but with string skipping and reverse direction (ensures
	 * last byte is included).
	 */
	hash = heap->hash_seed;
	step = (len >> DUK_USE_STRHASH_SKIP_SHIFT) + 1;
	for (off = len; off >= step; off -= step) {
		DUK_ASSERT(off >= 1);  /* off >= step, and step >= 1 */
		hash = ((hash << 5) + hash) ^ str[off - 1];
	}

	return hash;
}
#endif

#if 1
DUK_LOCAL duk_uint32_t duk__hashstring_fnv1(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;
	duk_size_t step;
	duk_size_t off;

	/* "FNV hash" from
	 * http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
	 * but with string skipping and reverse direction (ensures
	 * last byte is included).
	 */
	hash = heap->hash_seed;
	step = (len >> DUK_USE_STRHASH_SKIP_SHIFT) + 1;
	for (off = len; off >= step; off -= step) {
		DUK_ASSERT(off >= 1);  /* off >= step, and step >= 1 */
		hash = (hash * 16777619L) ^ str[off - 1];
	}

	return hash;
}
#endif

#if 1
DUK_LOCAL duk_uint32_t duk__hashstring_oaat1(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;
	duk_size_t step;
	duk_size_t off;

	/* "One-at-a-Time hash" from
	 * http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
	 * but with string skipping and reverse direction (ensures
	 * last byte is included).
	 */
	hash = heap->hash_seed;
	step = (len >> DUK_USE_STRHASH_SKIP_SHIFT) + 1;
	for (off = len; off >= step; off -= step) {
		DUK_ASSERT(off >= 1);  /* off >= step, and step >= 1 */
		hash += str[off - 1];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}
#endif

DUK_INTERNAL duk_uint32_t duk_heap_hashstring(duk_heap *heap, const duk_uint8_t *str, duk_size_t len) {
	duk_uint32_t hash;

#if 0
	hash = duk__hashstring_lua1(heap, str, len);
	hash = duk__hashstring_lua2(heap, str, len);
	hash = duk__hashstring_lua3(heap, str, len);
	hash = duk__hashstring_hybrid1(heap, str, len);
	hash = duk__hashstring_hybrid2(heap, str, len);
	hash = duk__hashstring_bernstein1a(heap, str, len);
	hash = duk__hashstring_bernstein1b(heap, str, len);
	hash = duk__hashstring_bernstein2a(heap, str, len);
	hash = duk__hashstring_bernstein2b(heap, str, len);
	hash = duk__hashstring_fnv1(heap, str, len);
	hash = duk__hashstring_oaat1(heap, str, len);
#endif

	hash = duk__hashstring_bernstein2b(heap, str, len);

#if defined(DUK_USE_STRHASH16)
	/* Truncate to 16 bits here, so that a computed hash can be compared
	 * against a hash stored in a 16-bit field.
	 */
	hash &= 0x0000ffffUL;
#endif
	return hash;
}
#endif  /* DUK_USE_STRHASH_DENSE */
