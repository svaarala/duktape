/*===
0
===*/

function test() {
    var arr = [ 'foo', 'bar', 'quux' ];
    arr.length = -0;
    print(arr.length);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
