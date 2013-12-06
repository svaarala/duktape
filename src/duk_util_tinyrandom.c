/*
 *  A tiny random number generator.
 *
 *  Currently used for Math.random().
 *
 *  http://www.woodmann.com/forum/archive/index.php/t-3100.html
 */

#include "duk_internal.h"

#define UPDATE_RND(rnd) do { \
		(rnd) += ((rnd) * (rnd)) | 0x05; \
		(rnd) = ((rnd) & 0xffffffffU);       /* if duk_uint32_t is exactly 32 bits, this is a NOP */ \
	} while (0)

#define RND_BIT(rnd)  ((rnd) >> 31)  /* only use the highest bit */

duk_uint32_t duk_util_tinyrandom_get_bits(duk_hthread *thr, duk_small_int_t n) {
	duk_small_int_t i;
	duk_uint32_t res = 0;
	duk_uint32_t rnd;

	rnd = thr->heap->rnd_state;

	for (i = 0; i < n; i++) {
		UPDATE_RND(rnd);
		res <<= 1;
		res += RND_BIT(rnd);
	}

	thr->heap->rnd_state = rnd;

	return res;
}

duk_double_t duk_util_tinyrandom_get_double(duk_hthread *thr) {
	duk_double_t t;
	duk_small_int_t n;
	duk_uint32_t rnd;

	/*
	 *  XXX: could make this a lot faster if we create the double memory
	 *  representation directly.  Feasible easily (must be uniform random).
	 */

	rnd = thr->heap->rnd_state;

	n = 53;  /* enough to cover the whole mantissa */
	t = 0.0;

	do {
		UPDATE_RND(rnd);
		t += RND_BIT(rnd);
		t /= 2.0;
	} while(--n);

	thr->heap->rnd_state = rnd;

	DUK_ASSERT(t >= (duk_double_t) 0.0);
	DUK_ASSERT(t < (duk_double_t) 1.0);

	return t;
}

