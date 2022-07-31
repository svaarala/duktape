function test() {
    var i;
    var x;

    x = 'foobarz'.repeat(1000);
    print(x);

    for (i = 0; i < 1e4; i++) {
        void x.split('z');
        void x.split('z');
        void x.split('z');
        void x.split('z');
        void x.split('z');
        void x.split('z');
        void x.split('z');
        void x.split('z');
        void x.split('z');
        void x.split('z');
    }
}

test();
