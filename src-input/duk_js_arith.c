/*
 *  Shared helpers for arithmetic operations
 */

#include "duk_internal.h"

/* Shared helper for Math.pow() and exponentiation operator. */
DUK_INTERNAL double duk_js_arith_pow(double x, double y) {
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
			 * the result here will be in the range ]-2,0], -1 indicates
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
