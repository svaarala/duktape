/*
 *  Duktape 2.x matches Node.js v6.9.1 where Buffer instances are Uint8Arrays,
 *  i.e. the internal class is "Uint8Array".
 */

/*---
{
    "custom": true
}
---*/

/*===
[object Function]
[object Object]
[object Uint8Array]
===*/

function nodejsBufferClassTest() {
    // Object.prototype.toString() prints "[object <classname>]" so use
    // it to indicate class.

    print(Object.prototype.toString.call(Buffer));
    print(Object.prototype.toString.call(Buffer.prototype));
    print(Object.prototype.toString.call(new Buffer(123)));
}

try {
    nodejsBufferClassTest();
} catch (e) {
    print(e.stack || e);
}
