/*
 *  https://github.com/svaarala/duktape/issues/103
 */

/*===
count: 2
===*/

function test() {
    var my_变量=1;
    my_变量++;
    print('count:', my_变量);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
