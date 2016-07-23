/*
 *  Node.js Buffer from an existing plain buffer
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
0
===*/

function nodejsBufferPlainBufTest() {
    var plain = Duktape.Buffer(100);
    print(typeof plain, plain.length);

    // XXX: as an initial first step treat like ArrayBuffer (= make a copy).
    // However, Node.js Buffer now accepts an ArrayBuffer and creates a new
    // Node.js buffer which *shares* the ArrayBuffer; to be updated.
    var buf = new Buffer(plain);
    print(typeof buf, buf.length);

    print(plain[10]);
    plain[10] = 0xa5;
    print(buf[10]);  // demonstrates underlying buffer is the same
}

try {
    nodejsBufferPlainBufTest();
} catch (e) {
    print(e.stack || e);
}
