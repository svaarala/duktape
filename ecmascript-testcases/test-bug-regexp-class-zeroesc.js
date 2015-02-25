/*
 *  https://github.com/svaarala/duktape/issues/122
 */

/*===
true
true
===*/

function test() {
    var re1 = /\0/;
    print(re1.test('\u0000'));

    var re2 = /[\0]/;
    print(re2.test('\u0000'));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
