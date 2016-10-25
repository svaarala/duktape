if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var src = 'x';

    while (src.length < 65536) {
        src = src + src;
    }
    src = '"' + src + '"';

    for (i = 0; i < 1e3; i++) {
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
