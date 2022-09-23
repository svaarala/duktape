function test() {
    var s;
    var chars = [];
    for (var i = 0; i < 256; i++) {
        chars.push(String.fromCodePoint(i));
    }

    for (var i = 0; i < 1e7; i++) {
        s = chars[i & 0x7f] + chars[(i >> 7) & 0x7f];
    }
}

test();
