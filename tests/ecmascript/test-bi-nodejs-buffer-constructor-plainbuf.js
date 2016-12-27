/*
 *  Node.js Buffer from an existing plain buffer
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
0
===*/

function nodejsBufferPlainBufTest() {
    var plain = createPlainBuffer(100);
    print(typeof plain, plain.length);

    // Plain buffers mimic Uint8Array, which gets treated as an initializer.
    // In other words, a copy is made.
    var buf = new Buffer(plain);
    print(typeof buf, buf.length);

    print(plain[10]);
    plain[10] = 0xa5;  // not visible in 'buf'
    print(buf[10]);
}

try {
    nodejsBufferPlainBufTest();
} catch (e) {
    print(e.stack || e);
}
