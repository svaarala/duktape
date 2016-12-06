/*
 *  https://github.com/svaarala/duktape/pull/1154
 */

/*===
number 5
===*/

function test() {
    var arr = [ 1, 2, 3 ];
    var ret = arr.push('foo', 'bar');
    print(typeof ret, ret);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
