function test() {
    var i;

    for (i = 0; i < 1e7; i++) {
        void Math.PI
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
