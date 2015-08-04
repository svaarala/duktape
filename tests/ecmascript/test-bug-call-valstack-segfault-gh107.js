/*
 *  https://github.com/svaarala/duktape/issues/107
 */

/*===
still here
===*/

function test() {
    Function().apply(1,Array(10000));
    print('still here');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
