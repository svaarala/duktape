/*---
{
    "custom": true
}
---*/

/*===
1e1000000 number Infinity
1e-1000000 number 0
1e1000000000 number Infinity
1e-1000000000 number 0
Error
Error
Error
Error
Error
Error
Error
Error
===*/

/* There is an implementation limit for the maximum exponent value
 * we're willing to parse.  There shouldn't be a limit in principle,
 * because one can represent a finite value, such as 1, with an
 * arbitrarily large or small exponent by combining it with a suitable
 * mantissa (e.g. "1" is equivalent to "100e-2", "100000e-5", etc).
 *
 * The implementation limit has been imposed to keep the exponent value
 * a normal integer (not a bigint).  Test for the current, non-compliant
 * behavior here.
 */

function test(exp_val) {
    t = '1e' + exp_val;

    try {
        num = Number(t);
        print(t, typeof num, num);
    } catch (e) {
        print(e.name);
    }
}

function expLimitTest() {
    // below the limit
    test('1000000');        // net value is 1e1000000 -> Infinity
    test('-1000000');       // net value is 1e-1000000 -> 0

    // just at the limit
    test('1000000000');        // Infinity
    test('-1000000000');       // 0

    // just over the current limit
    test('1000000001');    // net value should be 1e1000000001 -> Infinity; InternalError instead
    test('-1000000001');   // net value should be 1e-1000000001 -> 0; InternalError instead

    // over the current limit
    test('10000000000');
    test('-10000000000');
    test('100000000000000000000');
    test('-100000000000000000000');
    test('1000000000000000000000000000000');
    test('-1000000000000000000000000000000');
}

try {
    expLimitTest();
} catch (e) {
    print(e);
}
