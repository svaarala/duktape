function test() {
    var b = new Uint8Array(4096);
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

test();
