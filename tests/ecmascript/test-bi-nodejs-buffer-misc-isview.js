/*
 *  In Node.js v6.9.1 Buffers are Uint8Arrays, so that ArrayBuffer.isView()
 *  is true.
 */

/*===
true
===*/

function test() {
    var buf = new Buffer(10);
    print(ArrayBuffer.isView(buf));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
