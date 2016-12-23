/*
 *  ArrayBuffer from a plain buffer.
 */

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
object 100
object 100
0
165
object 0
object 100
165
188
===*/

function arrayBufferPlainBufTest() {
    // Duktape 2.x: plain buffer mimics Uint8Array.
    var plain = createPlainBuffer(100);
    print(typeof plain, plain.length);

    // Duktape 2.x: Object() coerces a plain buffer into an equivalent
    // Uint8Array, sharing same storage.
    var buf = Object(plain);
    print(typeof buf, buf.length);
    print(plain[10]);
    plain[10] = 0xa5;
    print(buf[10]);  // demonstrates underlying buffer is the same

    // This used to work in Duktape 1.x to create an ArrayBuffer with the
    // same underlying storage.  In Duktape 2.x the argument is ToNumber()
    // coerced which comes out as zero, creating a zero-length ArrayBuffer.
    var buf = new ArrayBuffer(plain);
    print(typeof buf, buf.byteLength);

    // In Duktape 2.x plain buffers have a virtual .buffer property which
    // returns a new ArrayBuffer sharing the same storage.
    buf = plain.buffer;
    print(typeof buf, buf.byteLength);
    print(plain[10]);
    plain[10] = 0xbc;
    print(new Uint8Array(buf)[10]);
}

try {
    arrayBufferPlainBufTest();
} catch (e) {
    print(e.stack || e);
}
