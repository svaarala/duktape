/*
 *  ArrayBuffer from an existing plain buffer
 */

/*---
{
    "custom": true
}
---*/

/*===
buffer 100
object 100
0
165
===*/

function arrayBufferPlainBufTest() {
    var plain = Duktape.Buffer(100);
    print(typeof plain, plain.length);

    var buf = new ArrayBuffer(plain);
    print(typeof buf, buf.length);

    print(plain[10]);
    plain[10] = 0xa5;
    print(buf[10]);  // demonstrates underlying buffer is the same
}

try {
    arrayBufferPlainBufTest();
} catch (e) {
    print(e.stack || e);
}
