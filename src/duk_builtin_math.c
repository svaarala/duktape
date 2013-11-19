/*
 *  Math built-ins
 */

#include "duk_internal.h"

/*
 *  Use static helpers which can work with math.h functions matching
 *  the following signatures. This is not portable if any of these math
 *  functions is actually a macro.
 */

typedef double (*one_arg_func)(double);
typedef double (*two_arg_func)(double, double);

static int math_minmax(duk_context *ctx, double initial, two_arg_func min_max) {
	int n = duk_get_top(ctx);
	int i;
	double res = initial;
	double t;

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
			res = DUK_DOUBLE_NAN;
		} else {
			res = min_max(res, t);
		}
	}

	duk_push_number(ctx, res);
	return 1;
}

static double fmin_fixed(double x, double y) {
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
	return fmin(x, y);
#else
	return (x < y ? x : y);
#endif
}

static double fmax_fixed(double x, double y) {
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
	return fmax(x, y);
#else
	return (x > y ? x : y);
#endif
}

static double round_fixed(double x) {
	/* Numbers half-way between integers must be rounded towards +Infinity,
	 * e.g. -3.5 must be rounded to -3 (not -4).  When rounded to zero, zero
	 * sign must be set appropriately.  E5.1 Section 15.8.2.15.
	 *
	 * Note that ANSI C round() is "round to nearest integer, away from zero",
	 * which is incorrect for negative values.  Here we make do with floor().
	 */

	int c = DUK_FPCLASSIFY(x);
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

	return floor(x + 0.5);
}

static double pow_fixed(double x, double y) {
	/* The ANSI C pow() semantics differ from Ecmascript.
	 *
	 * E.g. when x==1 and y is +/- infinite, the Ecmascript required
	 * result is NaN, while at least Linux pow() returns 1.
	 */

	int cy;

	cy = DUK_FPCLASSIFY(y);

	if (cy == DUK_FP_NAN) {
		goto ret_nan;
	}
	if (fabs(x) == 1.0 && cy == DUK_FP_INFINITE) {
		goto ret_nan;
	}

	return pow(x, y);

 ret_nan:
	return DUK_DOUBLE_NAN;
}

/* order must match constants in genbuiltins.py */
static one_arg_func one_arg_funcs[] = {
	fabs,
	acos,
	asin,
	atan,
	ceil,
	cos,
	exp,
	floor,
	log,
	round_fixed,
	sin,
	sqrt,
	tan
};

/* order must match constants in genbuiltins.py */
static two_arg_func two_arg_funcs[] = {
	atan2,
	pow_fixed
};

int duk_builtin_math_object_onearg_shared(duk_context *ctx) {
	int fun_idx = duk_get_magic(ctx);
	one_arg_func fun;

	DUK_ASSERT(fun_idx >= 0 && fun_idx < sizeof(one_arg_funcs) / sizeof(one_arg_func));
	fun = one_arg_funcs[fun_idx];
	duk_push_number(ctx, fun(duk_to_number(ctx, 0)));
	return 1;
}

int duk_builtin_math_object_twoarg_shared(duk_context *ctx) {
	int fun_idx = duk_get_magic(ctx);
	two_arg_func fun;

	DUK_ASSERT(fun_idx >= 0 && fun_idx < sizeof(two_arg_funcs) / sizeof(two_arg_func));
	fun = two_arg_funcs[fun_idx];
	duk_push_number(ctx, fun(duk_to_number(ctx, 0), duk_to_number(ctx, 1)));
	return 1;
}

int duk_builtin_math_object_max(duk_context *ctx) {
	return math_minmax(ctx, -DUK_DOUBLE_INFINITY, fmax_fixed);
}

int duk_builtin_math_object_min(duk_context *ctx) {
	return math_minmax(ctx, DUK_DOUBLE_INFINITY, fmin_fixed);
}

int duk_builtin_math_object_random(duk_context *ctx) {
	duk_push_number(ctx, (double) duk_util_tinyrandom_get_double((duk_hthread *) ctx));
	return 1;
}
