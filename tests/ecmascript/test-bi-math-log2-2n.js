/*
 *  While not required by ES2015, Math.log2(2^N) should return exactly N, at least
 *  with common C99 log2() implementations.
 */

/*===
positive exponents done
negative exponents done
===*/

function test() {
    var expect, x;

    for (expect = 0, x = 1; x !== 1/0; x *= 2, expect++) {
        if (Math.log2(x) != expect) {
            print('FAIL', x, Math.log2(x));
        }
    }
    print('positive exponents done');

    for (expect = 0, x = 1; x !== 0; x /= 2, expect--) {
        if (Math.log2(x) != expect) {
            print('FAIL', x, Math.log2(x));
        }
    }
    print('negative exponents done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
