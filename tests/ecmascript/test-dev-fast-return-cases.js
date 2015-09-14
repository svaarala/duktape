/*
 *  Exercise various code paths related to fast return handling.
 *
 *  Duktape executor supports two kinds of return:
 *
 *    - A slow return is implemented using longjmp() and handles all cases
 *      like label catchers, try-catch-finally blocks, etc.
 *
 *    - A fast return avoids the longjmp() but is limited to common cases
 *      only.
 *
 *  Longjmp has a moderate performance impact on normal platforms.  However
 *  it has a much larger relative impact when Duktape is compiled with
 *  Emscripten.
 */

/*===
testReturn1
returned 123
testReturn2
returned undefined
testReturn3
returned undefined
testForLoop1
returned 123
testTryCatch1
returned 123
testTryCatch2
returned 123
testTryFinally1
finally intercepted, return continues
returned 123
testTryFinally2
finally intercepted, replace return value
returned 321
testTryFinally3
returned 123
testTryFinally4
Error: canceled return
still here
returned 432
===*/

/* Basic case: fast return allowed. */
function testReturn1() {
    return 123;
}

/* Basic case, implicit return value (undefined).  Fast return allowed. */
function testReturn2() {
    return;
}

/* Basic case, implicit return value with no explicit return statement.
 * Fast return allowed.
 */
function testReturn3() {
}

/* Return from inside a label site (created by for-loop).  Fast return
 * is allowed for label catchers.
 */
function testForLoop1() {
    for (;;) {
        return 123;
    }
}

/* Return from inside the try block of a try-catch is currently not a
 * fast return.
 * XXX: could allow if there's no finally clause?
 */
function testTryCatch1() {
    try {
        return 123;
    } catch (e) {
    }
}

/* Return from inside the catch block of a try-catch is currently not a
 * fast return.
 * XXX: could allow if there's no finally clause?
 */
function testTryCatch2() {
    try {
        throw new Error('test');
    } catch (e) {
        return 123;
    }
}

/* Return from inside the try block of a try-finally is not a fast
 * return because the finally block intercepts the return (and can
 * even cancel it).
 */
function testTryFinally1() {
    try {
        return 123;
    } finally {
        print('finally intercepted, return continues');
    }
}

function testTryFinally2() {
    try {
        return 123;
    } finally {
        print('finally intercepted, replace return value');
        return 321;
    }
}

/* Return from inside the finally block of a try-finally is not a fast
 * return.
 * XXX: it could be, because there's no longer a chance of anyone
 * catching the return.
 */
function testTryFinally3() {
    try {
        ;
    } finally {
        return 123;
    }
}

function testTryFinally4() {
    // Here 'finally' cancels the return and replaces it with a thrown error.
    // The outer try-catch catches and neutralizes that, so that execution
    // resumes within the function normally.
    try {
        try {
            return 123;
        } finally {
            throw new Error('canceled return');
        }
    } catch (e) {
        print(e);
    }

    print('still here');
    return 432;
}

try {
    print('testReturn1');
    print('returned', testReturn1());

    print('testReturn2');
    print('returned', testReturn2());

    print('testReturn3');
    print('returned', testReturn3());

    print('testForLoop1');
    print('returned', testForLoop1());

    print('testTryCatch1');
    print('returned', testTryCatch1());

    print('testTryCatch2');
    print('returned', testTryCatch2());

    print('testTryFinally1');
    print('returned', testTryFinally1());

    print('testTryFinally2');
    print('returned', testTryFinally2());

    print('testTryFinally3');
    print('returned', testTryFinally3());

    print('testTryFinally4');
    print('returned', testTryFinally4());
} catch (e) {
    print(e);
}
