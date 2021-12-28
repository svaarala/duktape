if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var x;

    x = 'foobar\u{1f4a9}'.repeat(1000);
    console.log(x);

    for (i = 0; i < 1e4; i++) {
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
