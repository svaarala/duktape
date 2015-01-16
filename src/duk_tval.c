#include "duk_internal.h"

/* Set the duk_tval from a double, automatically detecting if an integer
 * representation can be used.  This needs to be optimized because it's
 * used a lot.
 *
 * Note special case for negative zero, which must be represented with
 * a double!
 */

#if defined(DUK_USE_FASTINT)
DUK_INTERNAL void duk_tval_set_number_double(duk_tval *tv, duk_double_t x) {
	duk_int64_t t = (duk_int64_t) x;
	duk_int32_t t_hi = (duk_int32_t) (t >> 32);

	if ((duk_double_t) t == x &&                 /* whole number, not necessarily in range */
	    (t_hi >= -0x8000 && t_hi <= 0x7fff) &&   /* in 48-bit range */
	    (t != 0 || !DUK_SIGNBIT(x))) {           /* non-zero or +0 (not -0) */
		DUK_TVAL_SET_NUMBER_FASTINT(tv, t);
#if 0  /* FIXME */
		fprintf(stderr, "set double to fastint: %lf -> %lld, hi=0x%08lx lo=0x%08lx - [ ",
		        x, (long long) t, (unsigned long) t_hi, (unsigned long) (t & 0xffffffffUL));
		{
			int i;
			for (i = 0; i < 8; i++) {
				fprintf(stderr, "%02x ", (int) ((duk_uint8_t *) tv)[i]);
			}
		}
		fprintf(stderr, "]\n");
#endif
	} else {
		DUK_TVAL_SET_NUMBER_DOUBLE(tv, x);
	}
}
#endif  /* DUK_USE_FASTINT */
