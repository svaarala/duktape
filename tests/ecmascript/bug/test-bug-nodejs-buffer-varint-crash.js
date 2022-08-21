/*===
RangeError
===*/

function test() {
    var buf = new Buffer('ABCDEFGH');

    // Value/offset "mixed up" caused segfault at some point in development.
    buf.writeIntBE(0, 0xdeadbeef, 4)
}

try {
    test();
} catch (e) {
    print(e.name);
}
