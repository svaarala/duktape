/*
 *  Number-to-string and string-to-number conversions.
 *
 *  Number-to-string conversion is based on a Dragon4 variant,
 *  with a fast path for radix 10 integers.  Limited big integer
 *  arithmetic is needed for guaranteeing that the conversion is
 *  correct and uses a minimum number of digits.
 *
 *  See: doc/number_conversion.txt.
 */

#include "duk_internal.h"

#define  IEEE_DOUBLE_EXP_BIAS  1023
#define  IEEE_DOUBLE_EXP_MIN   (-1022)   /* biased exp == 0 -> denormal, exp -1022 */

#define  DIGITCHAR(x)  duk_lc_digits[(x)]

/*
 *  Limited functionality bigint implementation.  Restricted to non-negative
 *  numbers with less than 32 * BI_MAX_PARTS bits, with the caller responsible
 *  for ensuring this is never exceeded.  Operations have been tailored for
 *  number conversion needs.
 */

#define  BI_MAX_PARTS  37  /* 37x32 = 1184 bits */

#ifdef DUK_USE_DEBUG
#define  BI_PRINT(name,x)  bi_print((name),(x))
#else
#define  BI_PRINT(name,x)
#endif

/* Current size is about 152 bytes. */
typedef struct {
	int n;
	uint32_t v[BI_MAX_PARTS];  /* low to high */
} duk_bigint;

#ifdef DUK_USE_DEBUG
static void bi_print(const char *name, duk_bigint *x) {
	char buf[1024];
	char *p = buf;
	int i;

	p += sprintf(p, "%p n=%d", (void *) x, x->n);
	if (x->n == 0) {
		p += sprintf(p, " 0");
	}
	for (i = x->n - 1; i >= 0; i--) {
		p += sprintf(p, " %08x", (unsigned int) x->v[i]);
	}

	DUK_DPRINT("%s: %s", name, buf);
}
#endif

#ifdef DUK_USE_ASSERTIONS
static int bi_is_normalized(duk_bigint *x) {
	return (x->n == 0) || (x->v[x->n - 1] != 0);
}
#endif

static void bi_normalize(duk_bigint *x) {
	int i;

	for (i = x->n - 1; i >= 0; i--) {
		if (x->v[i] != 0) {
			break;
		}
	}

	/* Note: if 'x' is zero, x->n becomes 0 here */
	x->n = i + 1;
	DUK_ASSERT(bi_is_normalized(x));
}

/* y <- x */
static void bi_copy(duk_bigint *x, duk_bigint *y) {
	int n;

	n = x->n;
	y->n = n;
	if (n == 0) {
		return;
	}
	memcpy((void *) y->v, (void *) x->v, (size_t) (sizeof(uint32_t) * n));
}

static void bi_set_small(duk_bigint *x, uint32_t v) {
	if (v == 0) {
		x->n = 0;
	} else {
		x->n = 1;
		x->v[0] = v;
	}
	DUK_ASSERT(bi_is_normalized(x));
}

/* z <- x+y */
static void bi_add(duk_bigint *x, duk_bigint *y, duk_bigint *z) {
	uint64_t tmp;
	int i, nx, ny;

	DUK_ASSERT(bi_is_normalized(x));
	DUK_ASSERT(bi_is_normalized(y));

	if (y->n > x->n) {
		duk_bigint *t;
		t = x; x = y; y = t;
	}
	/* now x->n >= y->n */

	nx = x->n; ny = y->n;
	tmp = 0;
	for (i = 0; i < nx; i++) {
		DUK_ASSERT(i < BI_MAX_PARTS);
		tmp += x->v[i];
		if (i < ny) {
			tmp += y->v[i];
		}
		z->v[i] = (uint32_t) (tmp & 0xffffffffU);
		tmp = tmp >> 32;
	}
	if (tmp != 0) {
		DUK_ASSERT(i < BI_MAX_PARTS);
		z->v[i++] = (uint32_t) tmp;
	}
	z->n = i;
	DUK_ASSERT(z->n < BI_MAX_PARTS);

	/* no need to normalize */
	DUK_ASSERT(bi_is_normalized(z));
}

/* z <- x-y, require x >= y => z >= 0 */
static void bi_sub(duk_bigint *x, duk_bigint *y, duk_bigint *z) {
	int nx, ny;
	int i;
	uint32_t tx, ty;
	int64_t tmp;

	DUK_ASSERT(bi_is_normalized(x));
	DUK_ASSERT(bi_is_normalized(y));

	nx = x->n;
	ny = y->n;
	/* ASSERT: x normalized, y normalized, nx >= ny */

	tmp = 0;
	for (i = 0; i < nx; i++) {
		tx = x->v[i];
		if (i < ny) {
			ty = y->v[i];
		} else {
			ty = 0;
		}
		tmp = (int64_t) tx - (int64_t) ty + tmp;
		z->v[i] = (uint32_t) (tmp & 0xffffffffU);
		tmp = tmp >> 32;  /* 0 or -1 */
	}
	/* ASSERT: tmp == 0 */

	z->n = i;
	bi_normalize(z);  /* need to normalize, may even cancel to 0 */
	DUK_ASSERT(bi_is_normalized(z));
}

/* z <- x*y */
static void bi_mul(duk_bigint *x, duk_bigint *y, duk_bigint *z) {
	int i, j, ny, nz;
	uint64_t tmp;

	DUK_ASSERT(bi_is_normalized(x));
	DUK_ASSERT(bi_is_normalized(y));

	nz = x->n + y->n;  /* max possible */
	DUK_ASSERT(nz < BI_MAX_PARTS);

	if (nz == 0) {
		z->n = 0;
		return;
	}

	memset((void *) z->v, 0, (size_t) (sizeof(uint32_t) * nz));
	z->n = nz;

	ny = y->n;
	for (i = 0; i < x->n; i++) {
		tmp = 0;
		for (j = 0; j < ny; j++) {
			tmp += (uint64_t) x->v[i] * (uint64_t) y->v[j] + z->v[i+j];
			z->v[i+j] = (uint32_t) (tmp & 0xffffffffU);
			tmp = tmp >> 32;
		}
		if (tmp > 0) {
			DUK_ASSERT(i + j < nz);
			DUK_ASSERT(i + j < BI_MAX_PARTS);
			z->v[i+j] += (uint32_t) tmp;
		}
	}

	bi_normalize(z);
	DUK_ASSERT(bi_is_normalized(z));
}

/* z <- x*y */
static void bi_mul_small(duk_bigint *x, uint32_t y, duk_bigint *z) {
	duk_bigint tmp;

	DUK_ASSERT(bi_is_normalized(x));

	/* FIXME: optimize */
	if (y == 0) {
		tmp.n = 0;
	} else {
		tmp.n = 1;
		tmp.v[0] = y;
	}
	bi_mul(x, &tmp, z);

	DUK_ASSERT(bi_is_normalized(z));
}

#if 0  /* FIXME: unused */
/* x <- x+y, use t as temp */
static void bi_add_copy(duk_bigint *x, duk_bigint *y, duk_bigint *t) {
	bi_add(x, y, t);
	bi_copy(t, x);
}
#endif

/* x <- x-y, use t as temp */
static void bi_sub_copy(duk_bigint *x, duk_bigint *y, duk_bigint *t) {
	bi_sub(x, y, t);
	bi_copy(t, x);
}

#if 0  /* FIXME: unused */
/* x <- x*y, use t as temp */
static void bi_mul_copy(duk_bigint *x, duk_bigint *y, duk_bigint *t) {
	bi_mul(x, y, t);
	bi_copy(t, x);
}
#endif

/* x <- x*y, use t as temp */
static void bi_mul_small_copy(duk_bigint *x, uint32_t y, duk_bigint *t) {
	bi_mul_small(x, y, t);
	bi_copy(t, x);
}

/* Return value: <0  <=>  x < y
 *                0  <=>  x == y
 *               >0  <=>  x > y
 */
static int bi_compare(duk_bigint *x, duk_bigint *y) {
	int i;
	int nx, ny;
	uint32_t tx, ty;

	DUK_ASSERT(bi_is_normalized(x));
	DUK_ASSERT(bi_is_normalized(y));

	nx = x->n;
	ny = y->n;
	if (nx > ny) {
		goto ret_gt;
	}
	if (nx < ny) {
		goto ret_lt;
	}
	for (i = nx - 1; i >= 0; i--) {
		tx = x->v[i];
		ty = y->v[i];

		if (tx > ty) {
			goto ret_gt;
		}
		if (tx < ty) {
			goto ret_lt;
		}
	}

	return 0;

 ret_gt:
	return 1;

 ret_lt:
	return -1;
}

static int bi_is_even(duk_bigint *x) {
	DUK_ASSERT(bi_is_normalized(x));
	return (x->n == 0) || ((x->v[0] & 0x01) == 0);
}

#if 0  /* FIXME: unused */
static int bi_is_zero(duk_bigint *x) {
	DUK_ASSERT(bi_is_normalized(x));
	return (x->n == 0);  /* this is the case for normalized numbers */
}
#endif

/* Bigint is 2^52.  Used to detect normalized IEEE double mantissa values
 * which are at the lowest edge (next floating point value downwards has
 * a different exponent).  The lowest mantissa has the form:
 *
 *     1000........000    (52 zeroes; only "hidden bit" is set)
 */
static int bi_is_2to52(duk_bigint *x) {
	DUK_ASSERT(bi_is_normalized(x));
	return (x->n == 2) && (x->v[0] == 0) && (x->v[1] == (1 << (52-32)));
}

#if 0  /* FIXME: not implemented, would be useful */
static void bi_shift(duk_bigint *x) {
}
#endif

/* x <- (1<<y) */
static void bi_twoexp(duk_bigint *x, int y) {
	int n, r;

	n = (y / 32) + 1;
	DUK_ASSERT(n > 0);
	r = y % 32;
	DUK_DPRINT("twoexp: y=%d, n=%d, r=%d", y, n, r);
	memset((void *) x->v, 0, sizeof(uint32_t) * n);
	x->n = n;
	x->v[n - 1] = (((uint32_t) 1) << r);
}

/*
 *  A Dragon4 number-to-string variant, based on:
 *
 *    Guy L. Steele Jr., Jon L. White: "How to Print Floating-Point
 *    Numbers Accurately"
 *
 *    Robert G. Burger, R. Kent Dybvig: "Printing Floating-Point
 *    Numbers Quickly and Accurately"
 *
 *  The current algorithm is based on Figure 1 of the Burger-Dybvig paper,
 *  i.e. the base implementation without logarithm estimation speedups;
 *  these would increase code footprint considerably.
 *
 *  b=2 is assumed (and optimized for); B is arbitrary.
 *
 *  FIXME: compile option to prefer speed over code footprint.
 */

/* Maximum number of digits generated. */
#define  MAX_OUTPUT_DIGITS  512  /* FIXME */

/* Number of extra digits for fixed-format rounding. */
#define  FIXED_FORMAT_EXTRA_DIGITS  1

/* Number and (minimum) of bigints in the nc_ctx structure. */
#define  NUMCONV_CTX_NUM_BIGINTS    7
#define  NUMCONV_CTX_BIGINTS_SIZE   (sizeof(duk_bigint) * NUMCONV_CTX_NUM_BIGINTS)

typedef struct {
	/* Currently about 7*152 = 1064 bytes.  The space for these
	 * duk_bigints is used also as a temporary buffer for generating
	 * the final string.  This is a bit awkard; a union would be
	 * more correct.
	 */
	/* FIXME: better reuse, eliminate 1? */
	duk_bigint f, r, s, mp, mm, t1, t2;

	double x;       /* input number */
	int req_digits; /* request number of output digits; 0 = free-format */
	int e;          /* exponent for 'f' */
	int B;          /* output radix */
	int k;          /* see algorithm */
	int low_ok;     /* see algorithm */
	int high_ok;    /* see algorithm */

	/* Buffer used for generated digits, values are in the range [0,B-1]. */
	char digits[MAX_OUTPUT_DIGITS];
	int count;  /* digit count */
} duk_numconv_stringify_ctx;

/* Note: computes with 'idx' in assertions, so caller beware.
 * 'idx' is preincremented, i.e. '1' on first call, because it
 * is more convenient for the caller.
 */
#define  DRAGON4_OUTPUT(nc_ctx,idx,x)  do { \
		DUK_ASSERT((idx) - 1 >= 0); \
		DUK_ASSERT((idx) - 1 < MAX_OUTPUT_DIGITS); \
		((nc_ctx)->digits[(idx) - 1]) = (x); \
	} while(0)

/* FIXME: very clunky */
size_t dragon4_format_uint(char *buf, unsigned int x, int radix) {
	char *p = buf;
	size_t len;
	int i;

	DUK_ASSERT(radix >= 2 && radix <= 36);

	/* output first in inverse order */
	for (;;) {
		int dig;
		int t;

		t = x / radix;
		dig = x - t * radix;
		x = t;

		*p++ = DIGITCHAR(dig);

		if (x == 0) {
			break;
		}
	}
	len = p - buf;

	/* then fix order: '12345' -> '54321' */
	for (i = 0; i < len / 2; i++) {
		/* even len: '1234' -> len=4 -> loop i=0,1
		 * odd len: '12345' -> len=5 -> loop i=0,1
		 */
		char tmp;

		tmp = buf[i];
		buf[i] = buf[len - i - 1];
		buf[len - i - 1] = tmp;
	}

	return len;
}

static void dragon4_convert_double(duk_numconv_stringify_ctx *nc_ctx) {
	/* FIXME: share duk_tval.h? */
	volatile union {
		double d;
		uint32_t v[2];
	} u;
	int exp;

	/*
	 *    seeeeeee eeeeffff ffffffff ffffffff ffffffff ffffffff ffffffff ffffffff
	 *       A        B        C        D        E        F        G        H
	 *
	 *    s       sign bit
	 *    eee...  exponent field
	 *    fff...  fraction
	 *
	 *    ieee value = 1.ffff... * 2^(e - 1023)  (normal)
	 *               = 0.ffff... * 2^(-1022)     (denormal)
	 *
	 *    algorithm v = f * b^e
	 */

	u.d = nc_ctx->x;

	/* FIXME: platform specific; need endianness define from duk_tval.h */

	nc_ctx->f.n = 2;
	nc_ctx->f.v[0] = u.v[0];
	nc_ctx->f.v[1] = u.v[1] & 0x000fffffU;

	exp = (u.v[1] >> 20) & 0x07ffU;
	if (exp == 0) {
		/* denormal */
		exp = IEEE_DOUBLE_EXP_MIN - 52;
	} else {
		/* normal: implicit leading 1-bit */
		nc_ctx->f.v[1] |= 0x00100000U;
		exp = exp - IEEE_DOUBLE_EXP_BIAS - 52;
	}
	
	nc_ctx->e = exp;
}

static void dragon4_prepare(duk_numconv_stringify_ctx *nc_ctx) {
#if 1
	/* Assume IEEE round-to-even, so that shorter encoding can be used
	 * when round-to-even would produce correct result.  By removing
	 * this check (and having low_ok == high_ok == 0) the results would
	 * still be accurate but in some cases longer than necessary.
	 */
	if (bi_is_even(&nc_ctx->f)) {
		DUK_DPRINT("f is even");
		nc_ctx->low_ok = 1;
		nc_ctx->high_ok = 1;
	} else {
		DUK_DPRINT("f is odd");
		nc_ctx->low_ok = 0;
		nc_ctx->high_ok = 0;
	}
#else
	/* Note: not honoring round-to-even should work but now generates incorrect
	 * results.  For instance, 1e23 serializes to "a000...", i.e. the first digit
	 * equals the radix (10).  Scaling stops one step too early in this case.
	 * Don't know why this is the case, but since this code path is unused, it
	 * doesn't matter.
	 */
	nc_ctx->low_ok = 0;
	nc_ctx->high_ok = 0;
#endif

	if (nc_ctx->e >= 0) {
		/* exponent non-negative (and thus not minimum exponent) */
		if (bi_is_2to52(&nc_ctx->f)) {
			/* (>= e 0) AND (= f (expt b (- p 1)))
			 *
			 * be <- (expt b e) == b^e
			 * be1 <- (* be b) == (expt b (+ e 1)) == b^(e+1)
			 * r <- (* f be1 2) == 2 * f * b^(e+1) ;  if b==2 -> f * b^(e+2)
			 * s <- (* b 2) ;  if b==2 -> 4
			 * m+ <- be1 == b^(e+1)
			 * m- <- be == b^e
			 * k <- 0
			 * B <- B
			 * low_ok <- round
			 * high_ok <- round
			 */

			DUK_DPRINT("non-negative exponent (not smallest exponent); "
			           "lowest mantissa value for this exponent -> "
			           "unequal gaps");

			bi_twoexp(&nc_ctx->t1, nc_ctx->e + 2);
			bi_mul(&nc_ctx->f, &nc_ctx->t1, &nc_ctx->r);
			bi_set_small(&nc_ctx->s, 4);
			bi_twoexp(&nc_ctx->mp, nc_ctx->e + 1);
			bi_twoexp(&nc_ctx->mm, nc_ctx->e);
		} else {
			/* (>= e 0) AND (not (= f (expt b (- p 1))))
			 *
			 * be <- (expt b e) == b^e
			 * r <- (* f be 2) == 2 * f * b^e ;  if b==2 -> f * b^(e+1)
			 * s <- 2
			 * m+ <- be == b^e
			 * m- <- be == b^e
			 * k <- 0
			 * B <- B
			 * low_ok <- round
			 * high_ok <- round
			 */

			DUK_DPRINT("non-negative exponent (not smallest exponent); "
			           "not lowest mantissa for this exponent -> "
			           "equal gaps");

			bi_twoexp(&nc_ctx->t1, nc_ctx->e + 1);
			bi_mul(&nc_ctx->f, &nc_ctx->t1, &nc_ctx->r);        /* r <- (* 2 f (expt b e)) = (* f (expt b (+ e 1))) */
			bi_set_small(&nc_ctx->s, 2);
			bi_twoexp(&nc_ctx->t1, nc_ctx->e);
			bi_copy(&nc_ctx->t1, &nc_ctx->mp);
			bi_copy(&nc_ctx->t1, &nc_ctx->mm);
		}
	} else {
		if (nc_ctx->e > IEEE_DOUBLE_EXP_MIN /*not minimum exponent*/ &&
		    bi_is_2to52(&nc_ctx->f) /* lowest mantissa for this exponent*/) {
			/* r <- (* f b 2) == (* f 4)
			 * s <- (* (expt b (- 1 e)) 2) == b^(1-e) * 2 ;  if b==2 -> b^(2-e)
			 * m+ <- b == 2
			 * m- <- 1
			 * k <- 0
			 * B <- B
			 * low_ok <- round
			 * high_ok <- round
			 */

			DUK_DPRINT("negative exponent; not minimum exponent and "
			           "lowest mantissa for this exponent -> "
			           "unequal gaps");

			bi_mul_small(&nc_ctx->f, 4, &nc_ctx->r);
			bi_twoexp(&nc_ctx->s, 2 - nc_ctx->e);
			bi_set_small(&nc_ctx->mp, 2);
			bi_set_small(&nc_ctx->mm, 1);
		} else {
			/* r <- (* f 2)
			 * s <- (* (expt b (- e)) 2) == b^(-e) * 2 ;  if b==2 -> b^(1-e)
			 * m+ <- b == 1
			 * m- <- 1
			 * k <- 0
			 * B <- B
			 * low_ok <- round
			 * high_ok <- round
			 */

			DUK_DPRINT("negative exponent; minimum exponent or not "
			           "lowest mantissa for this exponent -> "
			           "equal gaps");

			bi_mul_small(&nc_ctx->f, 2, &nc_ctx->r);
			bi_twoexp(&nc_ctx->s, 1 - nc_ctx->e);
			bi_set_small(&nc_ctx->mp, 1);
			bi_set_small(&nc_ctx->mm, 1);
		}
	}
}

static void dragon4_scale(duk_numconv_stringify_ctx *nc_ctx) {
	int k = 0;

	/* This is essentially the 'scale' algorithm, with recursion removed.
	 * Note that 'k' is either correct immediately, or will move in one
	 * direction in the loop.  There's no need to do the low/high checks
	 * on every round (like the Scheme algorithm does).
	 *
	 * The scheme algorithm finds 'k' and updates 's' simultaneously,
	 * while the logical algorithm finds 'k' with 's' having its initial
	 * value, after which 's' is updated separately (see the Burger-Dybvig
	 * paper, Section 3.1, steps 2 and 3).
	 */

	/* FIXME: this algorithm could be optimized quite a lot by using e.g.
	 * a logarithm based estimator for 'k' and performing B^n multiplication
	 * using a lookup table or using some bit-representation based exp
	 * algorithm.  Currently we just loop, with significant performance
	 * impact for very large and very small numbers.
	 */

	/* FIXME: optimize for the case when mp === mm.  This is a VERY likely case
	 * and saves on bigint arithmetic.
	 */

	DUK_DPRINT("scale: B=%d, low_ok=%d, high_ok=%d", nc_ctx->B, nc_ctx->low_ok, nc_ctx->high_ok);
	BI_PRINT("r(init)", &nc_ctx->r);
	BI_PRINT("s(init)", &nc_ctx->s);
	BI_PRINT("mp(init)", &nc_ctx->mp);
	BI_PRINT("mm(init)", &nc_ctx->mm);

	for (;;) {
		DUK_DPRINT("scale loop (inc k), k=%d", k);
		BI_PRINT("r", &nc_ctx->r);
		BI_PRINT("s", &nc_ctx->s);
		BI_PRINT("m+", &nc_ctx->mp);
		BI_PRINT("m-", &nc_ctx->mm);

		bi_add(&nc_ctx->r, &nc_ctx->mp, &nc_ctx->t1);  /* t1 = (+ r m+) */
		if (bi_compare(&nc_ctx->t1, &nc_ctx->s) >= (nc_ctx->high_ok ? 0 : 1)) {
			DUK_DPRINT("k is too low");
			/* r <- r
			 * s <- (* s B)
			 * m+ <- m+
			 * m- <- m-
			 * k <- (+ k 1)
			 */

			bi_mul_small_copy(&nc_ctx->s, nc_ctx->B, &nc_ctx->t1);
			k++;
		} else {
			break;
		}
	}

	/* k > 0 -> k was too low, and cannot be too high */
	if (k > 0) {
		goto skip_dec_k;
	}

	for (;;) {
		DUK_DPRINT("scale loop (dec k), k=%d", k);
		BI_PRINT("r", &nc_ctx->r);
		BI_PRINT("s", &nc_ctx->s);
		BI_PRINT("m+", &nc_ctx->mp);
		BI_PRINT("m-", &nc_ctx->mm);

		bi_add(&nc_ctx->r, &nc_ctx->mp, &nc_ctx->t1);  /* t1 = (+ r m+) */
		bi_mul_small(&nc_ctx->t1, nc_ctx->B, &nc_ctx->t2);   /* t2 = (* (+ r m+) B) */
		if (bi_compare(&nc_ctx->t2, &nc_ctx->s) <= (nc_ctx->high_ok ? -1 : 0)) {
			DUK_DPRINT("k is too high");
			/* r <- (* r B)
			 * s <- s
			 * m+ <- (* m+ B)
			 * m- <- (* m- B)
			 * k <- (- k 1)
			 */
			bi_mul_small_copy(&nc_ctx->r, nc_ctx->B, &nc_ctx->t1);
			bi_mul_small_copy(&nc_ctx->mp, nc_ctx->B, &nc_ctx->t1);
			bi_mul_small_copy(&nc_ctx->mm, nc_ctx->B, &nc_ctx->t1);
			k--;
		} else {
			break;
		}
	}

 skip_dec_k:

	DUK_DPRINT("final k: %d", k);
	nc_ctx->k = k;
}

static void dragon4_generate(duk_numconv_stringify_ctx *nc_ctx) {
	int tc1, tc2;
	int d;
	int count = 0;

	for (;;) {
		DUK_DPRINT("generate loop, count=%d, k=%d, B=%d, low_ok=%d, high_ok=%d",
		       count, nc_ctx->k, nc_ctx->B, nc_ctx->low_ok, nc_ctx->high_ok);
		BI_PRINT("r", &nc_ctx->r);
		BI_PRINT("s", &nc_ctx->s);
		BI_PRINT("m+", &nc_ctx->mp);
		BI_PRINT("m-", &nc_ctx->mm);

		/* (quotient-remainder (* r B) s) using a dummy subtraction loop */
		bi_mul_small(&nc_ctx->r, nc_ctx->B, &nc_ctx->t1);       /* t1 <- (* r B) */
		d = 0;
		for (;;) {
			if (bi_compare(&nc_ctx->t1, &nc_ctx->s) < 0) {
				break;
			}
			bi_sub_copy(&nc_ctx->t1, &nc_ctx->s, &nc_ctx->t2);  /* t1 <- t1 - s */
			d++;
		}
		bi_copy(&nc_ctx->t1, &nc_ctx->r);  /* r <- (remainder (* r B) s) */
		                                   /* d <- (quotient (* r B) s)   (in range 0...B-1) */
		DUK_DPRINT("-> d(quot)=%d", d);
		BI_PRINT("r(rem)", &nc_ctx->r);

		bi_mul_small_copy(&nc_ctx->mp, nc_ctx->B, &nc_ctx->t2); /* m+ <- (* m+ B) */
		bi_mul_small_copy(&nc_ctx->mm, nc_ctx->B, &nc_ctx->t2); /* m- <- (* m- B) */
		BI_PRINT("mp(upd)", &nc_ctx->mp);
		BI_PRINT("mm(upd)", &nc_ctx->mm);

		/* Terminating conditions.  For fixed width output, we just ignore the
		 * terminating conditions (and pretend that tc1 == tc2 == false).  The
		 * the current shortcut for fixed-format output is to generate a few
		 * extra digits and use rounding (with carry) to finish the output.
		 */

		if (nc_ctx->req_digits == 0) {
			/* free-form */
			tc1 = (bi_compare(&nc_ctx->r, &nc_ctx->mm) <= (nc_ctx->low_ok ? 0 : -1));

			bi_add(&nc_ctx->r, &nc_ctx->mp, &nc_ctx->t1);  /* t1 <- (+ r m+) */
			tc2 = (bi_compare(&nc_ctx->t1, &nc_ctx->s) >= (&nc_ctx->high_ok ? 0 : 1));

			DUK_DPRINT("tc1=%d, tc2=%d", tc1, tc2);
		} else {
			/* fixed-width */
			tc1 = 0;
			tc2 = 0;
		}

		/* Count is incremented before DRAGON4_OUTPUT() call on purpose.  This
		 * is taken into account by DRAGON4_OUTPUT() macro.
		 */
		count++;

		if (tc1) {
			if (tc2) {
				/* tc1 = true, tc2 = true */
				bi_mul_small(&nc_ctx->r, 2, &nc_ctx->t1);  /* FIXME: shift */
				if (bi_compare(&nc_ctx->t1, &nc_ctx->s) < 0) {  /* (< (* r 2) s) */
					DUK_DPRINT("tc1=true, tc2=true, 2r > s: output d --> %d (k=%d)", d, nc_ctx->k);
					DRAGON4_OUTPUT(nc_ctx, count, d);
				} else {
					DUK_DPRINT("tc1=true, tc2=true, 2r <= s: output d+1 --> %d (k=%d)", d + 1, nc_ctx->k);
					DRAGON4_OUTPUT(nc_ctx, count, d + 1);
				}
				break;
			} else {
				/* tc1 = true, tc2 = false */
				DUK_DPRINT("tc1=true, tc2=false: output d --> %d (k=%d)", d, nc_ctx->k);
				DRAGON4_OUTPUT(nc_ctx, count, d);
				break;
			}
		} else {
			if (tc2) {
				/* tc1 = false, tc2 = true */
				DUK_DPRINT("tc1=false, tc2=true: output d+1 --> %d (k=%d)", d + 1, nc_ctx->k);
				DRAGON4_OUTPUT(nc_ctx, count, d + 1);
				break;
			} else {
				/* tc1 = false, tc2 = false */

				DUK_DPRINT("tc1=false, tc2=false: output d --> %d (k=%d)", d, nc_ctx->k);
				DRAGON4_OUTPUT(nc_ctx, count, d);

				/* r <- r    (updated above: r <- (remainder (* r B) s)
				 * s <- s
				 * m+ <- m+  (updated above: m+ <- (* m+ B)
				 * m- <- m-  (updated above: m- <- (* m- B)
				 * B, low_ok, high_ok are fixed
				 */

				/* fall through and continue for-loop */
			}
		}

		if (nc_ctx->req_digits > 0 && count >= nc_ctx->req_digits) {
			DUK_DPRINT("digit count reached req_digits, end generate loop");
			break;
		}
	}  /* for */

	nc_ctx->count = count;

	DUK_DPRINT("generate finished");
}

static void dragon4_fixed_format_round(duk_numconv_stringify_ctx *nc_ctx) {
	int t;
	char *p;
	int roundup_limit;

	DUK_ASSERT(nc_ctx->req_digits >= FIXED_FORMAT_EXTRA_DIGITS);
	DUK_ASSERT(nc_ctx->count == nc_ctx->req_digits);

	/*
	 *  Round-up limit.
	 *
	 *  For even values, divides evenly, e.g. 10 -> roundup_limit=5.
	 *
	 *  For odd values, rounds up, e.g. 3 -> roundup_limit=2.
	 *  If radix is 3, 0/3 -> down, 1/3 -> down, 2/3 -> up.
	 */
	roundup_limit = (nc_ctx->B + 1) / 2;

	p = &nc_ctx->digits[nc_ctx->count - 1];
	if (*p >= roundup_limit) {
		DUK_DPRINT("fixed-format rounding carry required");
		/* carry */
		for (;;) {
			DUK_DPRINT("fixed-format rounding carry: B=%d, roundup_limit=%d, p=%p, digits=%p",
			           (int) nc_ctx->B, roundup_limit, (void *) p, (void *) nc_ctx->digits);
			p--;
			t = *p;
			DUK_DPRINT("digit before carry: %d", t);
			if (++t < nc_ctx->B) {
				DUK_DPRINT("rounding carry terminated");
				*p = t;
				break;
			}

			DUK_DPRINT("wraps, carry to next digit");
			*p = 0;
			if (p == &nc_ctx->digits[0]) {
				DUK_DPRINT("carry propagated to first digit -> special case handling");
				memmove((void *) (&nc_ctx->digits[1]),
				        (void *) (&nc_ctx->digits[0]),
				        (size_t) (sizeof(char) * nc_ctx->count));
				nc_ctx->digits[0] = 1;  /* don't increase 'count' */
				nc_ctx->k++;  /* position of highest digit changed */
				break;
			}
		}
	}

	/* 'erase' extra digits */
	nc_ctx->count -= FIXED_FORMAT_EXTRA_DIGITS;
	DUK_ASSERT(nc_ctx->count >= 1);
}

static void dragon4_convert_and_push(duk_numconv_stringify_ctx *nc_ctx, duk_context *ctx, int radix, int neg) {
	int k;
	int i;
	int pos;
	char *q;
	char *buf;

	/* Reuse bigint space in the context for string output, as there is more
	 * than enough space for that (>1kB at the moment).
	 *
	 * FIXME: currently ugly, use a union.
	 */
	DUK_ASSERT(NUMCONV_CTX_BIGINTS_SIZE >= 1024);  /* FIXME: what's maximum output size? */

	k = nc_ctx->k;
	buf = (char *) &nc_ctx->f;
	q = buf;

	if (neg) {
		*q++ = '-';
	}

	/* k == 0 -> first digit is in first fraction position (0.X) */
	if (k <= 0) {
		*q++ = '0';
		if (k < 0) {
			*q++ = '.';
			for (i = 0; i > k; i--) {
				*q++ = '0';
			}
		}
	}

	for (i = 0; i < nc_ctx->count; i++) {
		int dig = nc_ctx->digits[i];
		DUK_ASSERT(dig >= 0 && dig < nc_ctx->B);

		pos = k - i;
		if (pos == 0) {
			*q++ = '.';
		}
		*q++ = DIGITCHAR(dig);
	}

#if 1
	for (;; i++) {
		int pos = k - i;
		if (pos <= 0) {
			break;
		}
		*q++ = '0';
	}
#else
	/*
	 *  Exponent notation for non-base-10 numbers isn't specified in Ecmascript
	 *  specification, as it never explicitly turns up: non-decimal numbers can
	 *  only be formatted with Number.prototype.toString([radix]) and for that,
	 *  behavior is not explicitly specified.
	 *
	 *  Logical choices include formatting the exponent as decimal (e.g. binary
	 *  100000 as 1e+5) or in current radix (e.g. binary 100000 as 1e+101).
	 *  The Dragon4 algorithm (in the original paper) prints the exponent value
	 *  in the target radix B.  However, for radix values 15 and above, the
	 *  exponent separator 'e' is no longer easily parseable.  Consider, for
	 *  instance, the number "1.faecee+1c".
	 */

	pos = k - i;
	if (pos > 0) {
		size_t len;

		*q++ = 'e';
		*q++ = '+';
		len = dragon4_format_uint(q, (unsigned int) pos, radix);
		q += len;
	}
#endif

	duk_push_lstring(ctx, buf, q - buf);
}

/*
 *  Exposed number-to-string API
 */

void duk_numconv_stringify(duk_context *ctx, double x, int radix, int digits) {
	int c;
	int neg;
	unsigned int uval;
	duk_numconv_stringify_ctx nc_ctx_alloc;  /* large context; around 1kB now */
	duk_numconv_stringify_ctx *nc_ctx = &nc_ctx_alloc;

	/*
	 *  Handle special cases (NaN, infinity, zero).
	 */

	c = fpclassify(x);
	if (x < 0) {
		x = -x;
		neg = 1;
	} else {
		neg = 0;
	}

	if (c == FP_NAN) {
		duk_push_hstring_stridx(ctx, DUK_STRIDX_NAN);
		return;
	} else if (c == FP_INFINITE) {
		if (neg) {
			/* -Infinity */
			duk_push_hstring_stridx(ctx, DUK_STRIDX_MINUS_INFINITY);
		} else {
			/* Infinity */
			duk_push_hstring_stridx(ctx, DUK_STRIDX_INFINITY);
		}
		return;
	} else if (c == FP_ZERO) {
		/* zero sign is not printed -- FIXME: flag? */
		duk_push_hstring_stridx(ctx, DUK_STRIDX_ZERO);
		return;
	}

	/*
	 *  Handle integers in 32-bit range (that is, [-(2**32-1),2**32-1])
	 *  specially, as they're very likely for embedded programs.  This
	 *  is now done for all radix values.
	 *
	 *  FIXME: can save space by supporting radix 10 only and using
	 *  sprintf "%u".
	 */

	uval = (unsigned int) x;
	if (((double) uval) == x) {
		/* use bigint area as a temp */
		char *buf = (char *) (&nc_ctx->f);
		char *p = buf;

		DUK_ASSERT(NUMCONV_CTX_BIGINTS_SIZE >= 32 + 1);  /* max size: radix=2 + sign */
		if (neg) {
			*p++ = '-';
		}
		p += dragon4_format_uint(p, uval, radix);
		duk_push_lstring(ctx, buf, (size_t) (p - buf));
		return;
	}

	/*
	 *  Convert double from IEEE representation for conversion;
	 *  normal finite values have an implicit leading 1-bit.
	 */

	/* Would be nice to do this, but the context is 1-2 kilobytes and
	 * nothing should rely on it being zeroed.
	 */
#if 0
	memset((void *) nc_ctx, 0, sizeof(*nc_ctx));  /* slow init, do only for slow path cases */
#endif

	nc_ctx->x = x;
	nc_ctx->B = radix;
	if (digits > 0) {
		nc_ctx->req_digits = digits + FIXED_FORMAT_EXTRA_DIGITS;
	} else {
		nc_ctx->req_digits = 0;
	}

	dragon4_convert_double(nc_ctx);   /* -> sets 'f' and 'e' */
	BI_PRINT("f", &nc_ctx->f);
	DUK_DPRINT("e=%d", nc_ctx->e);

	/*
	 *  Preparation part (flonum->digits).
	 */

	dragon4_prepare(nc_ctx);  /* setup many variables in nc_ctx */

	DUK_DPRINT("after prepare:");
	BI_PRINT("r", &nc_ctx->r);
	BI_PRINT("s", &nc_ctx->s);
	BI_PRINT("mp", &nc_ctx->mp);
	BI_PRINT("mm", &nc_ctx->mm);

	/*
	 *  Scale part.
	 */

	dragon4_scale(nc_ctx);

	DUK_DPRINT("after scale; k=%d", nc_ctx->k);
	BI_PRINT("r", &nc_ctx->r);
	BI_PRINT("s", &nc_ctx->s);
	BI_PRINT("mp", &nc_ctx->mp);
	BI_PRINT("mm", &nc_ctx->mm);

	/*
	 *  Generate part.
	 */

	dragon4_generate(nc_ctx);

	if (digits > 0) {
		/* Perform fixed-format rounding. */
		dragon4_fixed_format_round(nc_ctx);
	}

	/*
	 *  Convert and push final string.
	 */

	dragon4_convert_and_push(nc_ctx, ctx, radix, neg);  /* FIXME: lots of options */
}

