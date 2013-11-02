/*
 *  String hash computation (interning).
 */

#include "duk_internal.h"

/* constants for duk_hashstring() */
#define  STRING_HASH_SHORTSTRING   4096
#define  STRING_HASH_MEDIUMSTRING  (256 * 1024)
#define  STRING_HASH_BLOCKSIZE     256

duk_uint32_t duk_heap_hashstring(duk_heap *heap, duk_uint8_t *str, duk_uint32_t len) {
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
	duk_uint32_t str_seed = heap->hash_seed ^ len;

	if (len <= STRING_HASH_SHORTSTRING) {
		return duk_util_hashbytes(str, len, str_seed);
	} else {
		duk_uint32_t hash;
		duk_uint32_t off;
		duk_uint32_t skip;

		if (len <= STRING_HASH_MEDIUMSTRING) {
			skip = 16 * STRING_HASH_BLOCKSIZE + STRING_HASH_BLOCKSIZE;
		} else {
			skip = 256 * STRING_HASH_BLOCKSIZE + STRING_HASH_BLOCKSIZE;
		}

		hash = duk_util_hashbytes(str, STRING_HASH_SHORTSTRING, str_seed);
		off = STRING_HASH_SHORTSTRING + (skip * (hash % 256)) / 256;

		/* FIXME: inefficient loop */
		while (off < len) {
			duk_uint32_t left = len - off;
			duk_uint32_t now = (left > STRING_HASH_BLOCKSIZE ? STRING_HASH_BLOCKSIZE : left);
			hash ^= duk_util_hashbytes(str + off, now, str_seed);
			off += skip;
		}

		return hash;
	}
}

