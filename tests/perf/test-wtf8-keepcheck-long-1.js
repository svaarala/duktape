function test() {
    var s1 = 'x'.repeat(1024 * 1024);
    var s2;

    for (var i = 0; i < 10000; i++) {
        s2 = String.fromCodePoint(i & 0x7f, (i >> 7) & 0x7f) + s1; // ASCII
    }
}

test();
