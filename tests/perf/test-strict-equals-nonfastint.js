/*
 *  Non-fastint equality comparison.
 */
function test() {
    var i, x, y;

    x = 1.1; y = 2.2;
    for (i = 0; i < 1e6; i++) {
        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);
        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);

        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);
        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);

        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);
        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);

        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);
        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);

        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);
        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);

        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);
        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);

        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);
        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);

        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);
        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);

        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);
        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);

        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);
        void (x === y); void (x === y); void (x === y); void (x === y); void (x === y);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
