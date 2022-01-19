if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var x;

    x = ('foobarx'.repeat(1000) + 'z').repeat(10);
    print(x);

    for (i = 0; i < 1e3; i++) {
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

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
