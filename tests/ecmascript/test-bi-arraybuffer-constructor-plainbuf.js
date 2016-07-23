/*
 *  ArrayBuffer from an existing plain buffer.
 */

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
===*/

function arrayBufferPlainBufTest() {
    // Duktape 2.x: plain buffer mimics ArrayBuffer.
    var plain = Duktape.Buffer(100);
    print(typeof plain, plain.length);

    // Duktape 2.x: Object() coerces a plain buffer into an equivalent
    // ArrayBuffer, sharing same storage.
    var buf = Object(plain);
    print(typeof buf, buf.length);

    print(plain[10]);
    plain[10] = 0xa5;
    print(buf[10]);  // demonstrates underlying buffer is the same

    // This used to work in Duktape 1.x to create an ArrayBuffer with the
    // same underlying storage.  In Duktape 2.x the argument is ToNumber()
    // coerced which comes out as zero, creating a zero-length ArrayBuffer.
    var buf = new ArrayBuffer(plain);
    print(typeof buf, buf.length);
}

try {
    arrayBufferPlainBufTest();
} catch (e) {
    print(e.stack || e);
}
