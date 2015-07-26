/*
 *  Subclassing Buffers.
 *
 *  Right now .slice() returns a new view instance which copies the
 *  internal prototype of the this binding (instead of using Buffer.prototype).
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
[object Buffer]
MyBuffer
MyNodejsBuffer
true
object
[object Buffer]
MyBuffer
MyNodejsBuffer
true
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
