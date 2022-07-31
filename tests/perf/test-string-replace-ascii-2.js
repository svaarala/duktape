function test() {
    var i;
    var x;

    x = 'foobarX'.repeat(1000);
    print(x);

    for (i = 0; i < 5e2; i++) {
        void x.replace(/X/g, 'ZZ');
        void x.replace(/X/g, 'ZZ');
        void x.replace(/X/g, 'ZZ');
        void x.replace(/X/g, 'ZZ');
        void x.replace(/X/g, 'ZZ');
        void x.replace(/X/g, 'ZZ');
        void x.replace(/X/g, 'ZZ');
        void x.replace(/X/g, 'ZZ');
        void x.replace(/X/g, 'ZZ');
        void x.replace(/X/g, 'ZZ');
    }
}

test();
