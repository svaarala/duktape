/*
 *  Subclassing Buffers.
 *
 *  Right now .slice() returns a new view instance which has the original
 *  Buffer.prototype as its internal prototype (changed from Duktape 1.x).
 */

/*---
{
    "custom": true
}
---*/

/*===
object
[object Uint8Array]
MyBuffer
MyNodejsBuffer
true
object
[object Uint8Array]
FGH
undefined
false
===*/

function slicePrototypeInheritanceTest() {
    var proto = {
        name: 'MyNodejsBuffer',
        toString: function () { return 'MyBuffer'; }
    };
    Object.setPrototypeOf(proto, Buffer.prototype);

    var b1 = new Buffer('ABCDEFGH');
    Object.setPrototypeOf(b1, proto);
    print(typeof b1);
    print(Object.prototype.toString.call(b1));
    print(String(b1));
    print(b1.name);
    print(Object.getPrototypeOf(b1) === proto);

    var b2 = b1.slice(5);
    print(typeof b2);
    print(Object.prototype.toString.call(b2));
    print(String(b2));
    print(b2.name);
    print(Object.getPrototypeOf(b2) === proto);
}

try {
    slicePrototypeInheritanceTest();
} catch (e) {
    print(e.stack || e);
}
