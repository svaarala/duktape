/*
 *  Bug in RegExp class dash parsing: https://github.com/svaarala/duktape/issues/1569
 */

/*===
true
true
true
true
===*/

function test() {
    var re;

    re = /^[a-zA-Z0-9_!#$%&'*+.^`|~-]+$/;
    print(re.test('Content-Type'));
    re = /^[a-zA-Z0-9_!#$%&'*+.^`|~\-]+$/;
    print(re.test('Content-Type'));

    re = /[-]/;
    print(re.test('-'));
    re = /[\-]/;
    print(re.test('-'));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
