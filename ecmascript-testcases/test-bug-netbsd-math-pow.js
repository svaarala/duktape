/*
 *  Duktape 0.9.0 has some math failures on NetBSD 6.0 x86.
 */

/*===
number Infinity nonzero
number -Infinity nonzero
number Infinity nonzero
number Infinity nonzero
number Infinity nonzero
===*/

function pr(x) {
    print(typeof x, x, (x === 0 ? (1 / x === Number.POSITIVE_INFINITY ? 'poszero' : 'negzero') : 'nonzero'));
}

function test() {
    // See E5.1 Section 15.8.2.13

    // Should be +Infinity but is -Infinity on NetBSD
    pr(Math.pow(+0, -0.000001));

    // "If x is -0 and y<0 and y is an odd integer, the result is -Infinity"
    // (works)
    pr(Math.pow(-0, -3));

    // When x is -0, and y<0, and y is not an odd integer, the result should
    // be +Infinity; these fail on NetBSD and provides -Infinity
    pr(Math.pow(-0, -4));
    pr(Math.pow(-0, -4.5));

    // Here again, since y is not an odd integer, the result should be
    // +Infinity; this works on NetBS too
    pr(Math.pow(-0, Number.NEGATIVE_INFINITY));
}

try {
    test();
} catch (e) {
    print(e);
}
