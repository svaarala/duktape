/*===
Infinity
===*/

/* Exponent overflow/underflow test; caused some valgrind issues. */

function buildNumber(exp) {
    var t;

    t = '1';
    while (exp-- > 0) { t += '0'; }
    return t;
}

function expOverflowUnderflowTest() {
    var str, radix, val;

    radix = 2;

    str = buildNumber(1024);
    val = parseInt(str, radix);
    print(val);
}

try {
    expOverflowUnderflowTest();
} catch (e) {
    print(e);
}
