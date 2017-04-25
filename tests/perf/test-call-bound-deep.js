function target() {
}
function test() {
    var bound = target;
    bound = bound.bind('dummy-this', 1);
    bound = bound.bind('dummy-this', 2);
    bound = bound.bind('dummy-this', 3);
    bound = bound.bind('dummy-this', 4);
    bound = bound.bind('dummy-this', 5);
    bound = bound.bind('dummy-this', 6);
    bound = bound.bind('dummy-this', 7);
    bound = bound.bind('dummy-this', 8);
    bound = bound.bind('dummy-this', 9);
    bound = bound.bind('dummy-this', 10);
    var i;
    for (i = 0; i < 1e5; i++) {
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
        void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12); void bound(11, 12);
    }
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
