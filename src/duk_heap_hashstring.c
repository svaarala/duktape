/*
 *  String hash computation (interning).
 */

#include "duk_internal.h"

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
