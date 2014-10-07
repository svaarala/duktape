/*
 *  Test limits on inner functions.
 */

/*===
test 1 ok
test 2 ok
test 3 ok
===*/

function test(n) {
    var src = '';

    src += '(function () {';
    for (i = 0; i < n; i++) {
        src += 'function innerfunc' + i + '() {} ';
    }
    src += '})';

    eval(src);
}

try {
    test(100);
    print('test 1 ok');
    test(200);
    print('test 2 ok');
    test(10000);
    print('test 3 ok');
} catch (e) {
    print(e);
}
