/*
 *  Large literals use up a lot of constants and even quite
 *  reasonable literals currently cause an "out of consts"
 *  error.
 */

/*===
test 1 ok
test 2 ok
test 3 ok
===*/

function test(n) {
    var src = '';

    src += '(function () {';
    src += 'var obj = {';
    for (i = 0; i < n; i++) {
        if (i > 0) { src += ', '; }
        src += 'key' + i + ': ' + '"val' + i + '"';
    }
    src += '}; })';

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
