/*
 *  Duktape uses "Buffer" as the class for Node.js Buffer instances.
 *  Node.js uses "Object".
 *
 *  This is useful e.g. when debugging because one can then easily see
 *  which values are Buffers.  Note that the same class number is used
 *  for Duktape.Buffer values.
 */

/*---
{
    "custom": true
}
---*/

/*===
[object Function]
[object Object]
[object Buffer]
[object Buffer]
===*/

function nodejsBufferClassTest() {
    // Object.prototype.toString() prints "[object <classname>]" so use
    // it to indicate class.

    print(Object.prototype.toString.call(Buffer));
    print(Object.prototype.toString.call(Buffer.prototype));
    print(Object.prototype.toString.call(new Buffer(123)));

    // Also Duktape.Buffer has class Buffer.

    print(Object.prototype.toString.call(new Duktape.Buffer(123)));
}

try {
    nodejsBufferClassTest();
} catch (e) {
    print(e.stack || e);
}
