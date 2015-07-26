function test() {
    var i;

    for (i = 0; i < 1e8; i++) {
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
