/*
 *  A tiny random number generator.
 *
 *  Currently used for Math.random().
 *
 *  http://www.woodmann.com/forum/archive/index.php/t-3100.html
 */

#include "duk_internal.h"

#if !defined(DUK_USE_GET_RANDOM_DOUBLE)

#define DUK__UPDATE_RND(rnd) do { \
		(rnd) += ((rnd) * (rnd)) | 0x05UL; \
		(rnd) = ((rnd) & 0xffffffffUL);       /* if duk_uint32_t is exactly 32 bits, this is a NOP */ \
	} while (0)

#define DUK__RND_BIT(rnd)  ((rnd) >> 31)  /* only use the highest bit */

#if 1
DUK_INTERNAL duk_double_t duk_util_tinyrandom_get_double(duk_hthread *thr) {
	duk_double_t t;
	duk_small_int_t n;
	duk_uint32_t rnd;

	rnd = thr->heap->rnd_state;

	n = 53;  /* enough to cover the whole mantissa */
	t = 0.0;

	do {
		DUK__UPDATE_RND(rnd);
		t += DUK__RND_BIT(rnd);
		t /= 2.0;
	} while (--n);

	thr->heap->rnd_state = rnd;

	DUK_ASSERT(t >= (duk_double_t) 0.0);
	DUK_ASSERT(t < (duk_double_t) 1.0);

	return t;
}
#else
/* Not much faster, larger footprint. */
DUK_INTERNAL duk_double_t duk_util_tinyrandom_get_double(duk_hthread *thr) {
	duk_double_t t;
	duk_small_int_t i;
	duk_uint32_t rnd;
	duk_uint32_t rndbit;
	duk_double_union du;

	rnd = thr->heap->rnd_state;

	du.ui[DUK_DBL_IDX_UI0] = 0x3ff00000UL;
	du.ui[DUK_DBL_IDX_UI1] = 0x00000000UL;

	DUK_ASSERT(DUK_DBL_IDX_UI0 ^ 1 == DUK_DBL_IDX_UI1);  /* Indices are 0,1 or 1,0. */
	DUK_ASSERT(DUK_DBL_IDX_UI1 ^ 1 == DUK_DBL_IDX_UI0);

	/* Fill double representation mantissa bits with random number.
	 * With the implicit leading 1-bit we get a value in [1,2[.
	 */
	for (i = 0; i < 52; i++) {
		DUK__UPDATE_RND(rnd);
		rndbit = DUK__RND_BIT(rnd);
		DUK_ASSERT((i >> 5) == 0 || (i >> 5) == 1);
		du.ui[DUK_DBL_IDX_UI1 ^ (i >> 5)] |= rndbit << (i & 0x1fUL);
	}

	thr->heap->rnd_state = rnd;

	t = du.d - 1.0;  /* Subtract implicit 1-bit to get [0,1[. */
	DUK_ASSERT(t >= (duk_double_t) 0.0);
	DUK_ASSERT(t < (duk_double_t) 1.0);

	return t;
}
#endif

#endif  /* !DUK_USE_GET_RANDOM_DOUBLE */
