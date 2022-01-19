if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var x;

    x = 'foobar\u{1f4a9}'.repeat(1000);
    print(x);

    for (i = 0; i < 5e2; i++) {
        void x.replace(/\ud83d/g, 'XX');
        void x.replace(/\ud83d/g, 'XX');
        void x.replace(/\ud83d/g, 'XX');
        void x.replace(/\ud83d/g, 'XX');
        void x.replace(/\ud83d/g, 'XX');
        void x.replace(/\ud83d/g, 'XX');
        void x.replace(/\ud83d/g, 'XX');
        void x.replace(/\ud83d/g, 'XX');
        void x.replace(/\ud83d/g, 'XX');
        void x.replace(/\ud83d/g, 'XX');
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
