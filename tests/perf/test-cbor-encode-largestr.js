function test() {
    var str;
    var i;

    str = 'xyzzy'.repeat(100000);
    for (i = 0; i < 1e4; i++) {
        void CBOR.encode(str);
    }
}

test();
