/*===
2
===*/

/* Exercise a special string-to-number corner case: the mantissa generated
 * is all 1s (53 + 1 digits of 1), and when the last digit is rounded, the
 * carry will ripple over the initial digit.  The exponent needs to be bumped
 * by one.  The final result is 2.
 */

try {
    print(Number('1.99999999999999989'));
} catch (e) {
    print(e);
}

/*===
Infinity
===*/

/* Same test, but the bump in exponent overflows the IEEE double range,
 * yielding Infinity.
 */

try {
    print(Number('1.79769313486231581e308'));
} catch (e) {
    print(e);
}
