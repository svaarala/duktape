function target() {
}
function test() {
    var bound = target.bind('dummy-this', 1, 2);
    var i;
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
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
