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

static int math_one_arg(duk_context *ctx, one_arg_func fun) {
	duk_push_number(ctx, fun(duk_to_number(ctx, 0)));
	return 1;
}

static int math_two_arg(duk_context *ctx, two_arg_func fun) {
	duk_push_number(ctx, fun(duk_to_number(ctx, 0), duk_to_number(ctx, 1)));
	return 1;
}

static int math_minmax(duk_context *ctx, double initial, two_arg_func min_max) {
	int n = duk_get_top(ctx);
	int i;
	double res = initial;
	double t;

	/*
	 *  Note: fmax() does not match the E5 semantics.  E5 requires
	 *  that if -any- input to Math.max() is a NAN, the result is a
	 *  NaN.  fmax() will return a NAN only if -both- inputs are NaN.
	 *  Same applies to fmin().
	 *
	 *  Note: every input value must be coerced with ToNumber(), even
	 *  if we know the result will be a NAN anyway: ToNumber() may have
	 *  side effects for which even order of evaluation matters.
	 */

	for (i = 0; i < n; i++) {
		t = duk_to_number(ctx, i);
		if (fpclassify(t) == FP_NAN || fpclassify(res) == FP_NAN) {
			/* Note: not normalized, but duk_push_number() will normalize */
			/* FIXME: best constant for NAN? */
			res = NAN;
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
		if (signbit(x) != 0 || signbit(y) != 0) {
			return -0.0;
		} else {
			return +0.0;
		}
	}
	return fmin(x, y);
}

static double fmax_fixed(double x, double y) {
	/* fmax() with args -0 and +0 is not guaranteed to return
	 * +0 as Ecmascript requires.
	 */
	if (x == 0 && y == 0) {
		if (signbit(x) == 0 || signbit(y) == 0) {
			return +0.0;
		} else {
			return -0.0;
		}
	}
	return fmax(x, y);
}


int duk_builtin_math_object_abs(duk_context *ctx) {
	return math_one_arg(ctx, fabs);
}

int duk_builtin_math_object_acos(duk_context *ctx) {
	return math_one_arg(ctx, acos);
}

int duk_builtin_math_object_asin(duk_context *ctx) {
	return math_one_arg(ctx, asin);
}

int duk_builtin_math_object_atan(duk_context *ctx) {
	return math_one_arg(ctx, atan);
}

int duk_builtin_math_object_atan2(duk_context *ctx) {
	return math_two_arg(ctx, atan2);
}

int duk_builtin_math_object_ceil(duk_context *ctx) {
	return math_one_arg(ctx, ceil);
}

int duk_builtin_math_object_cos(duk_context *ctx) {
	return math_one_arg(ctx, cos);
}

int duk_builtin_math_object_exp(duk_context *ctx) {
	return math_one_arg(ctx, exp);
}

int duk_builtin_math_object_floor(duk_context *ctx) {
	return math_one_arg(ctx, floor);
}

int duk_builtin_math_object_log(duk_context *ctx) {
	return math_one_arg(ctx, log);
}

int duk_builtin_math_object_max(duk_context *ctx) {
	return math_minmax(ctx, -((double) INFINITY), fmax_fixed);
}

int duk_builtin_math_object_min(duk_context *ctx) {
	return math_minmax(ctx, (double) INFINITY, fmin_fixed);
}

int duk_builtin_math_object_pow(duk_context *ctx) {
	return math_two_arg(ctx, pow);
}

int duk_builtin_math_object_random(duk_context *ctx) {
	duk_push_number(ctx, duk_util_tinyrandom_get_double((duk_hthread *) ctx));
	return 1;
}

int duk_builtin_math_object_round(duk_context *ctx) {
	/* FIXME: numbers must be rounded towards +Infinity, e.g.
	 * -3.5 must be rounded to -3 (not -4).
	 */
	return math_one_arg(ctx, round);
}

int duk_builtin_math_object_sin(duk_context *ctx) {
	return math_one_arg(ctx, sin);
}

int duk_builtin_math_object_sqrt(duk_context *ctx) {
	return math_one_arg(ctx, sqrt);
}

int duk_builtin_math_object_tan(duk_context *ctx) {
	return math_one_arg(ctx, tan);
}

