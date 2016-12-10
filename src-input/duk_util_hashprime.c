/*
 *  Round a number upwards to a prime (not usually the nearest one).
 *
 *  Uses a table of successive 32-bit primes whose ratio is roughly
 *  constant.  This keeps the relative upwards 'rounding error' bounded
 *  and the data size small.  A simple 'predict-correct' compression is
 *  used to compress primes to one byte per prime.  See genhashsizes.py
 *  for details.
 *
 *  The minimum prime returned here must be coordinated with the possible
 *  probe sequence steps in duk_hobject and duk_heap stringtable.
 */

#include "duk_internal.h"

/* Awkward inclusion condition: drop out of compilation if not needed by any
 * call site: object hash part or probing stringtable.
 */
#if defined(DUK_USE_HOBJECT_HASH_PART) || defined(DUK_USE_STRTAB_PROBE)

/* hash size ratio goal, must match genhashsizes.py */
#define DUK__HASH_SIZE_RATIO   1177  /* floor(1.15 * (1 << 10)) */

/* prediction corrections for prime list (see genhashsizes.py) */
DUK_LOCAL const duk_int8_t duk__hash_size_corrections[] = {
	17,  /* minimum prime */
	4, 3, 4, 1, 4, 1, 1, 2, 2, 2, 2, 1, 6, 6, 9, 5, 1, 2, 2, 5, 1, 3, 3, 3,
	5, 4, 4, 2, 4, 8, 3, 4, 23, 2, 4, 7, 8, 11, 2, 12, 15, 10, 1, 1, 5, 1, 5,
	8, 9, 17, 14, 10, 7, 5, 2, 46, 21, 1, 9, 9, 4, 4, 10, 23, 36, 6, 20, 29,
	18, 6, 19, 21, 16, 11, 5, 5, 48, 9, 1, 39, 14, 8, 4, 29, 9, 1, 15, 48, 12,
	22, 6, 15, 27, 4, 2, 17, 28, 8, 9, 4, 5, 8, 3, 3, 8, 37, 11, 15, 8, 30,
	43, 6, 33, 41, 5, 20, 32, 41, 38, 24, 77, 14, 19, 11, 4, 35, 18, 19, 41,
	10, 23, 16, 9, 2,
	-1
};

/* probe steps (see genhashsizes.py), currently assumed to be 32 entries long
 * (DUK_UTIL_GET_HASH_PROBE_STEP macro).
 */
DUK_INTERNAL duk_uint8_t duk_util_probe_steps[32] = {
	2, 3, 5, 7, 11, 13, 19, 31, 41, 47, 59, 67, 73, 79, 89, 101, 103, 107,
	109, 127, 137, 139, 149, 157, 163, 167, 173, 181, 191, 193, 197, 199
};

DUK_INTERNAL duk_uint32_t duk_util_get_hash_prime(duk_uint32_t size) {
	const duk_int8_t *p = duk__hash_size_corrections;
	duk_uint32_t curr;

	curr = (duk_uint32_t) *p++;
	for (;;) {
		duk_small_int_t t = (duk_small_int_t) *p++;
		if (t < 0) {
			/* may happen if size is very close to 2^32-1 */
			break;
		}

		/* prediction: portable variant using doubles if 64-bit values not available */
#if defined(DUK_USE_64BIT_OPS)
		curr = (duk_uint32_t) ((((duk_uint64_t) curr) * ((duk_uint64_t) DUK__HASH_SIZE_RATIO)) >> 10);
#else
		/* 32-bit x 11-bit = 43-bit, fits accurately into a double */
		curr = (duk_uint32_t) DUK_FLOOR(((double) curr) * ((double) DUK__HASH_SIZE_RATIO) / 1024.0);
#endif

		/* correction */
		curr += t;

		DUK_DDD(DUK_DDDPRINT("size=%ld, curr=%ld", (long) size, (long) curr));

		if (curr >= size) {
			return curr;
		}
	}
	return 0;
}

#endif  /* DUK_USE_HOBJECT_HASH_PART || DUK_USE_STRTAB_PROBE */
