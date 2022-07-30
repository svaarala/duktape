function test() {
    var b = new Buffer(4096);
    var i;

    print(typeof b);

    for (i = 0; i < 1e7; i++) {
        b[100] = 123;
        b[100] = 123;
        b[100] = 123;
        b[100] = 123;
        b[100] = 123;
        b[100] = 123;
        b[100] = 123;
        b[100] = 123;
        b[100] = 123;
        b[100] = 123;
    }
}

test();
