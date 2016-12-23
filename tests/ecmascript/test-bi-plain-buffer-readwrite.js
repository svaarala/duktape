/*@include util-buffer.js@*/

/*===
read/write coercion test
-1234 46
-256 0
-255 1
-1.6 255
-1.4 255
-1 255
-0.6 0
-0.4 0
-0 0
0 0
0.4 0
0.6 0
1 1
1.4 1
1.6 1
255 255
256 0
1234 210
NaN 0
Infinity 0
-Infinity 0
"123" 123
"130" 130
"-123" 133
"-130" 126
===*/

function readWriteCoercionTest() {
    var pb = createPlainBuffer('abcdefghijklmnop');

    [
      -1234, -256, -255, -1.6, -1.4, -1, -0.6, -0.4, -0,
      +0, 0.4, 0.6, 1, 1.4, 1.6, 255, 256, 1234,
      0/0, 1/0, -1/0, '123', '130', '-123', '-130'
    ].forEach(function (v) {
        pb[0] = v;
        print(Duktape.enc('jx', v), Duktape.enc('jx', pb[0]));
    });
}

try {
    print('read/write coercion test');
    readWriteCoercionTest();

} catch (e) {
    print(e.stack || e);
}
