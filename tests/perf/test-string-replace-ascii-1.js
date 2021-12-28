if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var x;

    x = 'foobarX'.repeat(1000);
    console.log(x);

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

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
