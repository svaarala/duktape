function test() {
    var t = function () {};
    var b;
    var i;

    for (i = 0; i < 2e5; i++) {
        b = t.bind('dummy-this', 1, 2);
        b = t.bind('dummy-this', 1, 2);
        b = t.bind('dummy-this', 1, 2);
        b = t.bind('dummy-this', 1, 2);
        b = t.bind('dummy-this', 1, 2);
        b = t.bind('dummy-this', 1, 2);
        b = t.bind('dummy-this', 1, 2);
        b = t.bind('dummy-this', 1, 2);
        b = t.bind('dummy-this', 1, 2);
        b = t.bind('dummy-this', 1, 2);
    }
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
