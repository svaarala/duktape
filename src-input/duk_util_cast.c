/*
 *  Cast helpers.
 *
 *  C99+ coercion is challenging portability-wise because out-of-range casts
 *  may invoke implementation defined or even undefined behavior.  See e.g.
 *  http://blog.frama-c.com/index.php?post/2013/10/09/Overflow-float-integer.
 *
 *  Provide explicit cast helpers which try to avoid implementation defined
 *  or undefined behavior.  These helpers can then be simplified in the vast
 *  majority of cases where the implementation defined or undefined behavior
 *  is not problematic.
 */

#include "duk_internal.h"

DUK_INTERNAL duk_int_t duk_double_to_int_t(duk_double_t x) {
#if defined(DUK_USE_ALLOW_UNDEFINED_BEHAVIOR)
	/* Real world solution: almost any practical platform will provide
	 * an integer value without any guarantees what it is (which is fine).
	 */
	return (duk_int_t) x;
#else
	/* Rely on specific NaN behavior for duk_double_{fmin,fmax}(): if
	 * either argument is a NaN, return the second argument.  This avoids
	 * a NaN-to-integer cast which is undefined behavior.
	 */
	return (duk_int_t) duk_double_fmin(duk_double_fmax(x, (double) DUK_INT_MIN), (double) DUK_INT_MAX);
#endif

#if 0
	/* Another solution which doesn't assume C99+ behavior for fmin()
	 * and fmax().
	 */
	if (DUK_ISNAN(x)) {
		return 0;
	} else {
		return (duk_int_t) DUK_FMIN(DUK_FMAX(x, (double) DUK_INT_MIN), (double) DUK_INT_MAX);
	}
#endif

#if 0
	/* C99+ solution: relies on specific fmin() and fmax() behavior in
	 * C99: if one argument is NaN but the other isn't, the non-NaN
	 * argument is returned.  Because the limits are non-NaN values,
	 * explicit NaN check is not needed.  This may not work on all legacy
	 * platforms.
	 */
	return (duk_int_t) DUK_FMIN(DUK_FMAX(x, (double) DUK_INT_MIN), (double) DUK_INT_MAX);
#endif
}

DUK_INTERNAL duk_uint_t duk_double_to_uint_t(duk_double_t x) {
#if defined(DUK_USE_ALLOW_UNDEFINED_BEHAVIOR)
	return (duk_uint_t) x;
#else
	return (duk_uint_t) duk_double_fmin(duk_double_fmax(x, (double) DUK_UINT_MIN), (double) DUK_UINT_MAX);
#endif

#if 0
	if (DUK_ISNAN(x)) {
		return 0;
	} else {
		return (duk_uint_t) DUK_FMIN(DUK_FMAX(x, (double) DUK_UINT_MIN), (double) DUK_UINT_MAX);
	}
#endif
#if 0
	return (duk_uint_t) DUK_FMIN(DUK_FMAX(x, (double) DUK_UINT_MIN), (double) DUK_UINT_MAX);
#endif
}

DUK_INTERNAL duk_int32_t duk_double_to_int32_t(duk_double_t x) {
#if defined(DUK_USE_ALLOW_UNDEFINED_BEHAVIOR)
	return (duk_int32_t) x;
#else
	return (duk_int32_t) duk_double_fmin(duk_double_fmax(x, (double) DUK_INT32_MIN), (double) DUK_INT32_MAX);
#endif

#if 0
	if (DUK_ISNAN(x)) {
		return 0;
	} else {
		return (duk_int32_t) DUK_FMIN(DUK_FMAX(x, (double) DUK_INT32_MIN), (double) DUK_INT32_MAX);
	}
#endif
#if 0
	return (duk_int32_t) DUK_FMIN(DUK_FMAX(x, (double) DUK_INT32_MIN), (double) DUK_INT32_MAX);
#endif
}

DUK_INTERNAL duk_uint32_t duk_double_to_uint32_t(duk_double_t x) {
#if defined(DUK_USE_ALLOW_UNDEFINED_BEHAVIOR)
	return (duk_uint32_t) x;
#else
	return (duk_uint32_t) duk_double_fmin(duk_double_fmax(x, (double) DUK_UINT32_MIN), (double) DUK_UINT32_MAX);
#endif

#if 0
	if (DUK_ISNAN(x)) {
		return 0;
	} else {
		return (duk_uint32_t) DUK_FMIN(DUK_FMAX(x, (double) DUK_UINT32_MIN), (double) DUK_UINT32_MAX);
	}
#endif
#if 0
	return (duk_uint32_t) DUK_FMIN(DUK_FMAX(x, (double) DUK_UINT32_MIN), (double) DUK_UINT32_MAX);
#endif
}

/* Largest IEEE double that doesn't round to infinity in the default rounding
 * mode.  The exact midpoint between (1 - 2^(-24)) * 2^128 and 2^128 rounds to
 * infinity, at least on x64.  This number is one double unit below that
 * midpoint.  See misc/float_cast.c.
 */
#define DUK__FLOAT_ROUND_LIMIT      340282356779733623858607532500980858880.0

/* Maximum IEEE float.  Double-to-float conversion above this would be out of
 * range and thus technically undefined behavior.
 */
#define DUK__FLOAT_MAX              340282346638528859811704183484516925440.0

DUK_INTERNAL duk_float_t duk_double_to_float_t(duk_double_t x) {
	/* Even a double-to-float cast is technically undefined behavior if
	 * the double is out-of-range.  C99 Section 6.3.1.5:
	 *
	 *   If the value being converted is in the range of values that can
	 *   be represented but cannot be represented exactly, the result is
	 *   either the nearest higher or nearest lower representable value,
	 *   chosen in an implementation-defined manner.  If the value being
	 *   converted is outside the range of values that can be represented,
	 *   the behavior is undefined.
	 */
#if defined(DUK_USE_ALLOW_UNDEFINED_BEHAVIOR)
	return (duk_float_t) x;
#else
	/* This assumes double NaN -> float NaN is considered "in range". */
	if (DUK_ISNAN(x)) {
		return (duk_float_t) x;
	} else {
		/* Assumes IEEE float/double limits. */
		if (x > DUK__FLOAT_MAX) {
			if (x <= DUK__FLOAT_ROUND_LIMIT) {
				return (duk_float_t) DUK__FLOAT_MAX;
			} else {
				return (duk_float_t) DUK_DOUBLE_INFINITY;
			}
		} else if (x < -DUK__FLOAT_MAX) {
			if (x >= -DUK__FLOAT_ROUND_LIMIT) {
				return (duk_float_t) -DUK__FLOAT_MAX;
			} else {
				return (duk_float_t) -DUK_DOUBLE_INFINITY;
			}
		} else {
			return (duk_float_t) x;
		}
	}
#endif
}
