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
		duk_double_union du1, du2;
		du1.d = x;
		du2.d = y;

		/* Already checked to be zero so these must hold, and allow us
		 * to check for "x is -0 or y is -0" by ORing the high parts
		 * for comparison.
		 */
		DUK_ASSERT(du1.ui[DUK_DBL_IDX_UI0] == 0 || du1.ui[DUK_DBL_IDX_UI0] == 0x80000000UL);
		DUK_ASSERT(du2.ui[DUK_DBL_IDX_UI0] == 0 || du2.ui[DUK_DBL_IDX_UI0] == 0x80000000UL);

		/* XXX: what's the safest way of creating a negative zero? */
		if ((du1.ui[DUK_DBL_IDX_UI0] | du2.ui[DUK_DBL_IDX_UI0]) != 0) {
			/* Enter here if either x or y (or both) is -0. */
			return -0.0;
		} else {
			return +0.0;
		}
	}
	return duk_double_fmin(x, y);
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
	return duk_double_fmax(x, y);
}

#if defined(DUK_USE_ES6)
DUK_LOCAL double duk__cbrt(double x) {
	/* cbrt() is C99.  To avoid hassling embedders with the need to provide a
	 * cube root function, we can get by with pow().  The result is not
	 * identical, but that's OK: ES2015 says it's implementation-dependent.
	 */

#if defined(DUK_CBRT)
	/* cbrt() matches ES2015 requirements. */
	return DUK_CBRT(x);
#else
	duk_small_int_t c = (duk_small_int_t) DUK_FPCLASSIFY(x);

	/* pow() does not, however. */
	if (c == DUK_FP_NAN || c == DUK_FP_INFINITE || c == DUK_FP_ZERO) {
		return x;
	}
	if (DUK_SIGNBIT(x)) {
		return -DUK_POW(-x, 1.0 / 3.0);
	} else {
		return DUK_POW(x, 1.0 / 3.0);
	}
#endif
}

DUK_LOCAL double duk__log2(double x) {
#if defined(DUK_LOG2)
	return DUK_LOG2(x);
#else
	return DUK_LOG(x) * DUK_DOUBLE_LOG2E;
#endif
}

DUK_LOCAL double duk__log10(double x) {
#if defined(DUK_LOG10)
	return DUK_LOG10(x);
#else
	return DUK_LOG(x) * DUK_DOUBLE_LOG10E;
#endif
}

DUK_LOCAL double duk__trunc(double x) {
#if defined(DUK_TRUNC)
	return DUK_TRUNC(x);
#else
	/* Handles -0 correctly: -0.0 matches 'x >= 0.0' but floor()
	 * is required to return -0 when the argument is -0.
	 */
	return x >= 0.0 ? DUK_FLOOR(x) : DUK_CEIL(x);
#endif
}
#endif  /* DUK_USE_ES6 */

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
DUK_LOCAL double duk__atan2_fixed(double x, double y) {
#if defined(DUK_USE_ATAN2_WORKAROUNDS)
	/* Specific fixes to common atan2() implementation issues:
	 * - test-bug-mingw-math-issues.js
	 */
	if (DUK_ISINF(x) && DUK_ISINF(y)) {
		if (DUK_SIGNBIT(x)) {
			if (DUK_SIGNBIT(y)) {
				return -2.356194490192345;
			} else {
				return -0.7853981633974483;
			}
		} else {
			if (DUK_SIGNBIT(y)) {
				return 2.356194490192345;
			} else {
				return 0.7853981633974483;
			}
		}
	}
#else
	/* Some ISO C assumptions. */
	DUK_ASSERT(DUK_ATAN2(DUK_DOUBLE_INFINITY, DUK_DOUBLE_INFINITY) == 0.7853981633974483);
	DUK_ASSERT(DUK_ATAN2(-DUK_DOUBLE_INFINITY, DUK_DOUBLE_INFINITY) == -0.7853981633974483);
	DUK_ASSERT(DUK_ATAN2(DUK_DOUBLE_INFINITY, -DUK_DOUBLE_INFINITY) == 2.356194490192345);
	DUK_ASSERT(DUK_ATAN2(-DUK_DOUBLE_INFINITY, -DUK_DOUBLE_INFINITY) == -2.356194490192345);
#endif

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
	duk__tan,
#if defined(DUK_USE_ES6)
	duk__cbrt,
	duk__log2,
	duk__log10,
	duk__trunc
#endif
#else  /* DUK_USE_AVOID_PLATFORM_FUNCPTRS */
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
	DUK_TAN,
#if defined(DUK_USE_ES6)
	duk__cbrt,
	duk__log2,
	duk__log10,
	duk__trunc
#endif
#endif  /* DUK_USE_AVOID_PLATFORM_FUNCPTRS */
};

/* order must match constants in genbuiltins.py */
DUK_LOCAL const duk__two_arg_func duk__two_arg_funcs[] = {
#if defined(DUK_USE_AVOID_PLATFORM_FUNCPTRS)
	duk__atan2_fixed,
	duk_js_arith_pow
#else
	duk__atan2_fixed,
	duk_js_arith_pow
#endif
};

DUK_INTERNAL duk_ret_t duk_bi_math_object_onearg_shared(duk_context *ctx) {
	duk_small_int_t fun_idx = duk_get_current_magic(ctx);
	duk__one_arg_func fun;
	duk_double_t arg1;

	DUK_ASSERT(fun_idx >= 0);
	DUK_ASSERT(fun_idx < (duk_small_int_t) (sizeof(duk__one_arg_funcs) / sizeof(duk__one_arg_func)));
	arg1 = duk_to_number(ctx, 0);
	fun = duk__one_arg_funcs[fun_idx];
	duk_push_number(ctx, (duk_double_t) fun((double) arg1));
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_twoarg_shared(duk_context *ctx) {
	duk_small_int_t fun_idx = duk_get_current_magic(ctx);
	duk__two_arg_func fun;
	duk_double_t arg1;
	duk_double_t arg2;

	DUK_ASSERT(fun_idx >= 0);
	DUK_ASSERT(fun_idx < (duk_small_int_t) (sizeof(duk__two_arg_funcs) / sizeof(duk__two_arg_func)));
	arg1 = duk_to_number(ctx, 0);  /* explicit ordered evaluation to match coercion semantics */
	arg2 = duk_to_number(ctx, 1);
	fun = duk__two_arg_funcs[fun_idx];
	duk_push_number(ctx, (duk_double_t) fun((double) arg1, (double) arg2));
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_max(duk_context *ctx) {
	return duk__math_minmax(ctx, -DUK_DOUBLE_INFINITY, duk__fmax_fixed);
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_min(duk_context *ctx) {
	return duk__math_minmax(ctx, DUK_DOUBLE_INFINITY, duk__fmin_fixed);
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_random(duk_context *ctx) {
	duk_push_number(ctx, (duk_double_t) DUK_UTIL_GET_RANDOM_DOUBLE((duk_hthread *) ctx));
	return 1;
}

#if defined(DUK_USE_ES6)
DUK_LOCAL duk_double_t duk__to_number_fabs(duk_context *ctx, duk_idx_t idx) {
	return DUK_FABS(duk_to_number(ctx, idx));
}

DUK_INTERNAL duk_ret_t duk_bi_math_object_hypot(duk_context *ctx) {
	/*
	 *  E6 Section 20.2.2.18: Math.hypot
	 *
	 *  - If no arguments are passed, the result is +0.
	 *  - If any argument is +inf, the result is +inf.
	 *  - If any argument is -inf, the result is +inf.
	 *  - If no argument is +inf or -inf, and any argument is NaN, the result is
	 *    NaN.
	 *  - If all arguments are either +0 or -0, the result is +0.
	 */

	duk_idx_t nargs;
	duk_idx_t i;
	duk_bool_t found_nan;
	duk_double_t max;
	duk_double_t sum, summand;
	duk_double_t comp, prelim;
	duk_double_t t;
	duk_double_t res;

	nargs = duk_get_top(ctx);

	/* Find the highest value.  Also ToNumber() coerces. */
	max = 0.0;
	found_nan = 0;
	for (i = 0; i < nargs; i++) {
		t = duk__to_number_fabs(ctx, i);
		if (DUK_ISNAN(t)) {
			found_nan = 1;
		} else {
			max = duk_double_fmax(max, t);
		}
	}

	/* Early return cases. */
	if (max == DUK_DOUBLE_INFINITY) {
		res = DUK_DOUBLE_INFINITY;
		goto done;
	} else if (found_nan) {
		res = DUK_DOUBLE_NAN;
		goto done;
	} else if (max == 0.0) {
		/* Otherwise we'd divide by zero. */
		res = max;
		goto done;
	}

	/* Use Kahan summation and normalize to the highest value to minimize
	 * floating point rounding error and avoid overflow.
	 *
	 * https://en.wikipedia.org/wiki/Kahan_summation_algorithm
	 */
	sum = 0.0;
	comp = 0.0;
	for (i = 0; i < nargs; i++) {
		DUK_ASSERT(duk_is_number(ctx, i));  /* coercion above guarantees */
		t = duk__to_number_fabs(ctx, i) / max;
		summand = (t * t) - comp;
		prelim = sum + summand;
		comp = (prelim - sum) - summand;
		sum = prelim;
	}
	res = (duk_double_t) DUK_SQRT(sum) * max;
	goto done;

 done:
	duk_push_number(ctx, res);
	return 1;
}
#endif  /* DUK_USE_ES6 */

#endif  /* DUK_USE_MATH_BUILTIN */
