#ifndef DUK_NUMCONV_H_INCLUDED
#define DUK_NUMCONV_H_INCLUDED

/*
 *  Number-to-string conversion.  The semantics of these is very tightly
 *  bound with the Ecmascript semantics required for call sites.
 */

/* Output a specified number of digits instead of using the shortest
 * form.  Used for toPrecision() and toFixed().
 */
#define  DUK_NUMCONV_FLAG_FIXED_FORMAT         (1 << 0)

/* Force exponential format.  Used for toExponential(). */
#define  DUK_NUMCONV_FLAG_FORCE_EXP            (1 << 1)

/* If number would need zero padding (for whole number part), use
 * exponential format instead.  E.g. if input number is 12300, 3
 * digits are generated ("123"), output "1.23e+4" instead of "12300".
 * Used for toPrecision().
 */
#define  DUK_NUMCONV_FLAG_NO_ZERO_PAD          (1 << 2)

/* Digit count indicates number of fractions (i.e. an absolute
 * digit index instead of a relative one).  Used together with
 * DUK_NUMCONV_FLAG_FIXED_FORMAT for toFixed().
 */
#define  DUK_NUMCONV_FLAG_FRACTION_DIGITS      (1 << 3)

void duk_numconv_stringify(duk_context *ctx, double x, int radix, int digits, int flags);

#endif  /* DUK_NUMCONV_H_INCLUDED */

