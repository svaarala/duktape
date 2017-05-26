function target() {
}
function test() {
    var bound = target.bind('dummy-this', 1, 2);
    var i;

    var t1 = Date.now();

    for (i = 0; i < 1e5; i++) {
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
        void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4); void bound(3, 4);
    }

    var t2 = Date.now();
    print((1e5 * 100 / (t2 - t1)) + ' calls per millisecond');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
