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
            console.log('FAIL', x, Math.log2(x));
        }
    }
    console.log('positive exponents done');

    for (expect = 0, x = 1; x !== 0; x /= 2, expect--) {
        if (Math.log2(x) != expect) {
            console.log('FAIL', x, Math.log2(x));
        }
    }
    console.log('negative exponents done');
}

try {
    test();
} catch (e) {
    console.log(e.stack || e);
}
