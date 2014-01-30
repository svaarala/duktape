/*===
0 Infinity
NaN
===*/

/* An empty string ToNumber() coerces to +0 (this is explicitly specified).
 * A string which cannot be interpreted as a StringNumericLiteral must
 * produce a NaN.
 *
 * A string containing just a U+0000 character is easily confused with an
 * empty string in C code, so this testcase ensures that doesn't happen.
 */

try {
    print(+'', 1/(+''));
    print(+'\u0000');
} catch (e) {
    print(e);
}
