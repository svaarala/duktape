if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var x;

    x = ('foobar\ucafe'.repeat(1000) + '\u{1f4a9}').repeat(10);
    print(x);

    for (i = 0; i < 1e3; i++) {
        void x.split('\u{1f4a9}');
        void x.split('\u{1f4a9}');
        void x.split('\u{1f4a9}');
        void x.split('\u{1f4a9}');
        void x.split('\u{1f4a9}');
        void x.split('\u{1f4a9}');
        void x.split('\u{1f4a9}');
        void x.split('\u{1f4a9}');
        void x.split('\u{1f4a9}');
        void x.split('\u{1f4a9}');
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
