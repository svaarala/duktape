/*
 *  Comma expression as LHS.
 */

/*===
2
0
===*/

function test() {
    var a = [];
    var b = [];
    var a_orig = a;
    var b_orig = b;

    /* Assignment must go to a_orig */
    (b = a), b[1] = 123;
    print(a_orig.length);
    print(b_orig.length);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
