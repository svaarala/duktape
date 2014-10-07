/*
 *  Exercise various code paths related to fast return handling.
 */

/* XXX: incomplete, finish when fast return implemented. */

/*===
returned 123
returned undefined
returned undefined
returned 123
===*/

/* Basic case, call to test1() is from Ecmascript, so a fast return is
 * allowed.
 */
function testReturn1() {
    return 123;
}

/* Basic case, implicit return value (undefined). */
function testReturn2() {
    return;
}

/* Basic case, implicit return value with no explicit return statement. */
function testReturn3() {
}

/* Return from inside a label site (created by for-loop) */
function testForLoop1() {
    for (;;) {
        return 123;
    }
}
try {
    print('returned', testReturn1());
    print('returned', testReturn2());
    print('returned', testReturn3());
    print('returned', testForLoop1());
} catch (e) {
    print(e);
}
