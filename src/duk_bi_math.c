/*
 *  Math built-ins
 */

#include "duk_internal.h"

#if defined(DUK_USE_MATH_BUILTIN)

/*
 *  Use static helpers which can work with math.h functions matching
 *  the following signatures. This is not portable if any of these math
 *  functions is actually a macro.
 *
 *  Typing here is intentionally 'double' wherever values interact with
 *  the standard library APIs.
 */

typedef double (*duk__one_arg_func)(double);
typedef double (*duk__two_arg_func)(double, double);

DUK_LOCAL duk_ret_t duk__math_minmax(duk_context *ctx, duk_double_t initial, duk__two_arg_func min_max) {
	duk_idx_t n = duk_get_top(ctx);
	duk_idx_t i;
	duk_double_t res = initial;
	duk_double_t t;

	/*
	 *  Note: fmax() does not match the E5 semantics.  E5 requires
	 *  that if -any- input to Math.max() is a NaN, the result is a
	 *  NaN.  fmax() will return a NaN only if -both- inputs are NaN.
	 *  Same applies to fmin().
	 *
	 *  Note: every input value must be coerced with ToNumber(), even
	 *  if we know the result will be a NaN anyway: ToNumber() may have
	 *  side effects for which even order of evaluation matters.
	 */

	for (i = 0; i < n; i++) {
		t = duk_to_number(ctx, i);
		if (DUK_FPCLASSIFY(t) == DUK_FP_NAN || DUK_FPCLASSIFY(res) == DUK_FP_NAN) {
			/* Note: not normalized, but duk_push_number() will normalize */
			res = (duk_double_t) DUK_DOUBLE_NAN;
		} else {
			res = (duk_double_t) min_max(res, (double) t);
		}
	}

	duk_push_number(ctx, res);
	return 1;
}

DUK_LOCAL double duk__fmin_fixed(double x, double y) {
	/* fmin() with args -0 and +0 is not guaranteed to return
	 * -0 as Ecmascript requires.
	 */
	if (x == 0 && y == 0) {
		/* XXX: what's the safest way of creating a negative zero? */
		if (DUK_SIGNBIT(x) != 0 || DUK_SIGNBIT(y) != 0) {
			return -0.0;
		} else {
			return +0.0;
		}
	}
#ifdef DUK_USE_MATH_FMIN
	return DUK_FMIN(x, y);
#else
	return (x < y ? x : y);
#endif
}

DUK_LOCAL double duk__fmax_fixed(double x, double y) {
	/* fmax() with args -0 and +0 is not guaranteed to return
	 * +0 as Ecmascript requires.
	 */
	if (x == 0 && y == 0) {
		if (DUK_SIGNBIT(x) == 0 || DUK_SIGNBIT(y) == 0) {
			return +0.0;
		} else {
			return -0.0;
		}
	}
#ifdef DUK_USE_MATH_FMAX
	return DUK_FMAX(x, y);
#else
	return (x > y ? x : y);
#endif
}

DUK_LOCAL double duk__round_fixed(double x) {
	/* Numbers half-way between integers must be rounded towards +Infinity,
	 * e.g. -3.5 must be rounded to -3 (not -4).  When rounded to zero, zero
	 * sign must be set appropriately.  E5.1 Section 15.8.2.15.
	 *
	 * Note that ANSI C round() is "round to nearest integer, away from zero",
	 * which is incorrect for negative values.  Here we make do with floor().
	 */

	duk_small_int_t c = (duk_small_int_t) DUK_FPCLASSIFY(x);
	if (c == DUK_FP_NAN || c == DUK_FP_INFINITE || c == DUK_FP_ZERO) {
		return x;
	}

	/*
	 *  x is finite and non-zero
	 *
	 *  -1.6 -> floor(-1.1) -> -2
	 *  -1.5 -> floor(-1.0) -> -1  (towards +Inf)
	 *  -1.4 -> floor(-0.9) -> -1
	 *  -0.5 -> -0.0               (special case)
	 *  -0.1 -> -0.0               (special case)
	 *  +0.1 -> +0.0               (special case)
	 *  +0.5 -> floor(+1.0) -> 1   (towards +Inf)
	 *  +1.4 -> floor(+1.9) -> 1
	 *  +1.5 -> floor(+2.0) -> 2   (towards +Inf)
	 *  +1.6 -> floor(+2.1) -> 2
	 */

	if (x >= -0.5 && x < 0.5) {
		/* +0.5 is handled by floor, this is on purpose */
		if (x < 0.0) {
			return -0.0;
		} else {
			return +0.0;
		}
	}

	return DUK_FLOOR(x + 0.5);
}

DUK_LOCAL double duk__pow_fixed(double x, double y) {
	/* The ANSI C pow() semantics differ from Ecmascript.
	 *
	 * E.g. when x==1 and y is +/- infinite, the Ecmascript required
	 * result is NaN, while at least Linux pow() returns 1.
	 */

	duk_small_int_t cx, cy, sx;

	DUK_UNREF(cx);
	DUK_UNREF(sx);
	cy = (duk_small_int_t) DUK_FPCLASSIFY(y);

	if (cy == DUK_FP_NAN) {
		goto ret_nan;
	}
	if (DUK_FABS(x) == 1.0 && cy == DUK_FP_INFINITE) {
		goto ret_nan;
	}
#if defined(DUK_USE_POW_NETBSD_WORKAROUND)
	/* See test-bug-netbsd-math-pow.js: NetBSD 6.0 on x86 (at least) does not
	 * correctly handle some cases where x=+/-0.  Specific fixes to these
	 * here.
	 */
	cx = (duk_small_int_t) DUK_FPCLASSIFY(x);
	if (cx == DUK_FP_ZERO && y < 0.0) {
		sx = (duk_small_int_t) DUK_SIGNBIT(x);
		if (sx == 0) {
			/* Math.pow(+0,y) should be Infinity when y<0.  NetBSD pow()
			 * returns -Infinity instead when y is <0 and finite.  The
			 * if-clause also catches y == -Infinity (which works even
			 * without the fix).
			 */
			return DUK_DOUBLE_INFINITY;
		} else {
			/* Math.pow(-0,y) where y<0 should be:
			 *   - -Infinity if y<0 and an odd integer
			 *   - Infinity otherwise
			 * NetBSD pow() returns -Infinity for all finite y<0.  The
			 * if-clause also catches y == -Infinity (which works even
			 * without the fix).
			 */

			/* fmod() return value has same sign as input (negative) so
			 * the result here will be in the range ]-2,0], 1 indicates
			 * odd.  If x is -Infinity, NaN is returned and the odd check
			 * always concludes "not odd" which results in desired outcome.
			 */
			double tmp = DUK_FMOD(y, 2);
			if (tmp == -1.0) {
				return -DUK_DOUBLE_INFINITY;
			} else {
				/* Not odd, or y == -Infinity */
				return DUK_DOUBLE_INFINITY;
			}
		}
	}
#endif
	return DUK_POW(x, y);

 ret_nan:
	return DUK_DOUBLE_NAN;
}

/* Wrappers for calling standard math library methods.  These may be required
 * on platforms where one or more of the math built-ins are defined as macros
 * or inline functions and are thus not suitable to be used as function pointers.
 */
#if defined(DUK_USE_AVOID_PLATFORM_FUNCPTRS)
DUK_LOCAL double duk__fabs(double x) {
	return DUK_FABS(x);
}
DUK_LOCAL double duk__acos(double x) {
	return DUK_ACOS(x);
}
DUK_LOCAL double duk__asin(double x) {
	return DUK_ASIN(x);
}
DUK_LOCAL double duk__atan(double x) {
	return DUK_ATAN(x);
}
DUK_LOCAL double duk__ceil(double x) {
	return DUK_CEIL(x);
}
DUK_LOCAL double duk__cos(double x) {
	return DUK_COS(x);
}
DUK_LOCAL double duk__exp(double x) {
	return DUK_EXP(x);
}
DUK_LOCAL double duk__floor(double x) {
	return DUK_FLOOR(x);
}
DUK_LOCAL double duk__log(double x) {
	return DUK_LOG(x);
}
DUK_LOCAL double duk__sin(double x) {
	return DUK_SIN(x);
}
DUK_LOCAL double duk__sqrt(double x) {
	return DUK_SQRT(x);
}
DUK_LOCAL double duk__tan(double x) {
	return DUK_TAN(x);
}
DUK_LOCAL double duk__atan2(double x, double y) {
	return DUK_ATAN2(x, y);
}
#endif  /* DUK_USE_AVOID_PLATFORM_FUNCPTRS */

/* order must match constants in genbuiltins.py */
DUK_LOCAL const duk__one_arg_func duk__one_arg_funcs[] = {
#if defined(DUK_USE_AVOID_PLATFORM_FUNCPTRS)
	duk__fabs,
	duk__acos,
	duk__asin,
	duk__atan,
	duk__ceil,
	duk__cos,
	duk__exp,
	duk__floor,
	duk__log,
	duk__round_fixed,
	duk__sin,
	duk__sqrt,
	duk__tan
#else
	DUK_FABS,
	DUK_ACOS,
	DUK_ASIN,
	DUK_ATAN,
	DUK_CEIL,
	DUK_COS,
	DUK_EXP,
	DUK_FLOOR,
	DUK_LOG,
	duk__round_fixed,
	DUK_SIN,
	DUK_SQRT,
	DUK_TAN
#endif
};

/* order must match constants in genbuiltins.py */
DUK_LOCAL const duk__two_arg_func duk__two_arg_funcs[] = {
#if defined(DUK_USE_AVOID_PLATFORM_FUNCPTRS)
	duk__atan2,
	duk__pow_fixed
#else
	DUK_ATAN2,
	duk__pow_fixed
#endif
};

DUK_INTERNAL duk_ret_t duk_bi_math_object_onearg_shared(duk_context *ctx) {
	duk_small_int_t fun_idx = duk_get_current_magic(ctx);
	duk__one_arg_func fun;

	DUK_ASSERT(fun_idx >= 0);
	DUK_ASSERT(fun_idx < (duk_small_int_t) (sizeof(duk__one_arg_funcs) / sizeof(duk__one_arg_func)));
	fun = duk__one_arg_funcs[fun_idx];
	duk_push_number(ctx, (duk_double_t) fun((double) duk_to_number(ctx, 0)));
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_twoarg_shared(duk_context *ctx) {
	duk_small_int_t fun_idx = duk_get_current_magic(ctx);
	duk__two_arg_func fun;

	DUK_ASSERT(fun_idx >= 0);
	DUK_ASSERT(fun_idx < (duk_small_int_t) (sizeof(duk__two_arg_funcs) / sizeof(duk__two_arg_func)));
	fun = duk__two_arg_funcs[fun_idx];
	duk_push_number(ctx, (duk_double_t) fun((double) duk_to_number(ctx, 0), (double) duk_to_number(ctx, 1)));
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_max(duk_context *ctx) {
	return duk__math_minmax(ctx, -DUK_DOUBLE_INFINITY, duk__fmax_fixed);
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_min(duk_context *ctx) {
	return duk__math_minmax(ctx, DUK_DOUBLE_INFINITY, duk__fmin_fixed);
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_random(duk_context *ctx) {
	duk_push_number(ctx, (duk_double_t) duk_util_tinyrandom_get_double((duk_hthread *) ctx));
	return 1;
}

#else  /* DUK_USE_MATH_BUILTIN */

/* A stubbed built-in is useful for e.g. compilation torture testing with BCC. */

DUK_INTERNAL duk_ret_t duk_bi_math_object_onearg_shared(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNIMPLEMENTED_ERROR;
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_twoarg_shared(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNIMPLEMENTED_ERROR;
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_max(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNIMPLEMENTED_ERROR;
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_min(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNIMPLEMENTED_ERROR;
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_random(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNIMPLEMENTED_ERROR;
}

#endif  /* DUK_USE_MATH_BUILTIN */
