/*===
1e+23
1e+23
1e+23
===*/

/* This integer constant parsed incorrectly in some version
 * and would print, incorrectly:
 *
 *   1.0000000000000001e+23
 *
 * Looking at the IEEE representations:
 *
 *   $ python
 *   Python 2.7.3 (default, Aug  1 2012, 05:14:39)
 *   [GCC 4.6.3] on linux2
 *   Type "help", "copyright", "credits" or "license" for more information.
 *   >>> import struct
 *   >>> struct.pack('>d', 1e+23).encode('hex')
 *   '44b52d02c7e14af6'
 *   >>> struct.pack('>d', 1.0000000000000001e+23).encode('hex')
 *   '44b52d02c7e14af7'
 *
 * It's apparent that the rounding of the last digit is incorrect.
 * The Dragon4 generator output (when a large number of binary digits
 * is requested explicitly) is:
 *
 *    generated digits; k=77,                                        .-- digit considered for rounding
 *    digits=                                                        v
 *        10101001 01101000 00010110 00111111 00001010 01010111 10110100 00000000 00000000 00000000 00000000 00000000 ... (zeroes indefinitely)
 *        ^                                                         ^
 *        `-- hidden bit                                            `-- last mantissa digit
 *
 * The problem here is that the number is apparently exactly halfway between
 * the two IEEE representations; the current rounding will only consider the
 * immedate '1' digit and round up.  This case is not trivial to fix because
 * we don't know, based on just the one rounding digit, that the number is
 * exactly halfway and we should round to even.
 */

try {
    print(parseInt('0000100000000000000000000000', 10));
    print(parseFloat('1e23', 10));
    print(parseFloat('1e+23', 10));
} catch (e) {
    print(e);
}
