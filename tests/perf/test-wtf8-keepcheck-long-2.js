function test() {
    var s1 = 'x'.repeat(1024 * 1024);
    var s2;

    for (var i = 0; i < 10000; i++) {
        s2 = s1 + String.fromCodePoint(0x100 + i); // ASCII followed by non-ASCII
    }
}

test();
