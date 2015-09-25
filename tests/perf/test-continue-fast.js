function test() {
    var i;
    for (i = 0; i < 1e8; i++) {
        do {
            continue;
        } while (0);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
