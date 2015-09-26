if (typeof print !== 'function') { print = console.log; }

function test() {
    var b = new Buffer(4096);
    var i;

    print(typeof b);

    for (i = 0; i < 1e7; i++) {
        void b[100];
        void b[100];
        void b[100];
        void b[100];
        void b[100];
        void b[100];
        void b[100];
        void b[100];
        void b[100];
        void b[100];
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
