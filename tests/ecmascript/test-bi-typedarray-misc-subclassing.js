/*
 *  Subclassing ArrayBuffer, DataView, or TypedArray views.
 *
 *  Right now .subarray() returns a new view instance which copies the
 *  internal prototype of the this binding (instead of using a default
 *  prototype based on type, e.g. Uint8Array.prototype).
 *
 *  This is probably not the preferred behavior, but test for current behavior.
 */

/*---
{
    "custom": true
}
---*/

/* Custom because current behavior differs from e.g. V8. */

/*===
object
[object Uint32Array]
MyBuffer
MyUint32Array
true
object
[object Uint32Array]
MyBuffer
MyUint32Array
true
===*/

function subarrayPrototypeInheritanceTest() {
    var buf = new ArrayBuffer(16);

    // Custom prototype inherits from Uint32Array.prototype.
    var proto = {
        name: 'MyUint32Array',
        toString: function () { return 'MyBuffer'; }
    };
    Object.setPrototypeOf(proto, Uint32Array.prototype);

    var v1 = new Uint32Array();
    Object.setPrototypeOf(v1, proto);
    print(typeof v1);
    print(Object.prototype.toString.call(v1));
    print(String(v1));
    print(v1.name);
    print(Object.getPrototypeOf(v1) === proto);

    var v2 = v1.subarray(0);
    print(typeof v2);
    print(Object.prototype.toString.call(v2));
    print(String(v2));
    print(v2.name);
    print(Object.getPrototypeOf(v2) === proto);
}

try {
    subarrayPrototypeInheritanceTest();
} catch (e) {
    print(e.stack || e);
}
