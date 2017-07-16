function test() {
    var i;
    var t1 = Date.now();
    for (i = 0; i < 1e6; i++) {
        // Use a RegExp constructor call rather than a literal to ensure the
        // RegExp is compiled on every loop.
        var re = new RegExp('[\\u0000-\\uffff]', '');
    }
    print(((Date.now() - t1) / 1e6) + ' ms/test');
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
