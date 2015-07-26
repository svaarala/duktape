/*
 *  ArrayBuffer.isView()
 */

/*---
{
    "custom": true
}
---*/

/*===
ArrayBuffer isView() test
function
function
false
0 undefined false
1 object false
2 boolean false
3 number false
4 string false
5 object false
6 object false
7 buffer false
8 object false
9 object false
10 object false
11 object true
12 object true
13 object true
14 object true
15 object true
16 object true
17 object true
18 object true
19 object true
20 object true
21 object true
22 object true
23 object true
24 object true
25 object true
26 object true
27 object true
28 object true
29 object true
30 object true
===*/

function arrayBufferIsViewTest() {
    print(typeof ArrayBuffer);
    print(typeof ArrayBuffer.isView);
    print(ArrayBuffer.isView());

    var b1 = Duktape.dec('hex', '414243');
    var b2 = new Duktape.Buffer(b1);
    var b3 = new Buffer('ABCDEFGH');
    var b4 = new ArrayBuffer(16);  // ensure compatible with alignment
    var v0 = new DataView(b4);
    var v1 = new Int8Array(b4);
    var v2 = new Uint8Array(b4);
    var v3 = new Uint8ClampedArray(b4);
    var v4 = new Int16Array(b4);
    var v5 = new Uint16Array(b4);
    var v6 = new Int32Array(b4);
    var v7 = new Uint32Array(b4);
    var v8 = new Float32Array(b4);
    var v9 = new Float64Array(b4);
    var w0 = new Int8Array(v0);
    var w1 = new Int8Array(v1);
    var w2 = new Int8Array(v2);
    var w3 = new Int8Array(v3);
    var w4 = new Int8Array(v4);
    var w5 = new Int8Array(v5);
    var w6 = new Int8Array(v6);
    var w7 = new Int8Array(v7);
    var w8 = new Int8Array(v8);
    var w9 = new Int8Array(v9);
    [
        undefined, null, true, 123, 'foo', { foo: 'bar' }, [ 'foo', 'bar' ],
        b1, b2, b3, b4,
        v0, v1, v2, v3, v4, v5, v6, v7, v8, v9,
        w0, w1, w2, w3, w4, w5, w6, w7, w8, w9
    ].forEach(function (v, idx) {
        try {
            print(idx, typeof v, ArrayBuffer.isView(v));
        } catch (e) {
            print(idx, typeof v, e);
        }
    });
}

try {
    print('ArrayBuffer isView() test');
    arrayBufferIsViewTest();
} catch (e) {
    print(e.stack || e);
}
