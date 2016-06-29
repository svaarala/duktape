/*
 *  Subclassing ArrayBuffer, DataView, or TypedArray views.
 *
 *  Right now .subarray() returns a new view instance whose internal
 *  prototype is set to the initial prototype matching the result
 *  type (e.g. original value of Uint8Array.prototype).
 */

/*---
{
    "custom": true
}
---*/

/*===
object
[object Uint32Array]
MyBuffer
MyUint32Array
true
object
[object Uint32Array]
[object Uint32Array]
undefined
false
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
