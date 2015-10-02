if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var src = '123 + myval';
    var myval = 234;

    for (i = 0; i < 1e5; i++) {
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
