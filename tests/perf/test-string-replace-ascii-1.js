function test() {
    var i;
    var x;

    x = 'foobarX'.repeat(1000);
    print(x);

    for (i = 0; i < 1e5; i++) {
        void x.replace('X', 'ZZ');
        void x.replace('X', 'ZZ');
        void x.replace('X', 'ZZ');
        void x.replace('X', 'ZZ');
        void x.replace('X', 'ZZ');
        void x.replace('X', 'ZZ');
        void x.replace('X', 'ZZ');
        void x.replace('X', 'ZZ');
        void x.replace('X', 'ZZ');
        void x.replace('X', 'ZZ');
    }
}

test();
