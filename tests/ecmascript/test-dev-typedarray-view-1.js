/*
 *  Simple view testing.
 */

/*---
{
    "custom": true
}
---*/

/*===
[object ArrayBuffer]
undefined 16 undefined
[object Uint32Array]
4 16 0
[object Uint32Array]
3 12 4
false
true
true
|01010101020202020303030304040404|
|020202020303030304040404|
===*/

function test() {
    var b = new ArrayBuffer(16);
    print(Object.prototype.toString.call(b));
    print(b.length, b.byteLength, b.byteOffset);

    var v = new Uint32Array(b);
    print(Object.prototype.toString.call(v));
    print(v.length, v.byteLength, v.byteOffset);

    var w = v.subarray(1)
    print(Object.prototype.toString.call(w));
    print(w.length, w.byteLength, w.byteOffset);

    print(ArrayBuffer.isView(b));
    print(ArrayBuffer.isView(v));
    print(ArrayBuffer.isView(w));

    v[0] = 0x01010101
    v[1] = 0x02020202
    v[2] = 0x03030303
    v[3] = 0x04040404

    print(Duktape.enc('jx', v));
    print(Duktape.enc('jx', w));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
