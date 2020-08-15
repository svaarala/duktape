if (typeof print !== 'function') { print = console.log; }

function mkObj(n) {
    var res = { foo: 123 };
    while (--n > 0) {
        res = Object.create(res);
        void res.noSuchProperty;  // simulate a _Finalizer lookup
    }
    return res;
}

function test() {
    for (var i = 0; i < 100; i++) {
        void mkObj(3000);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
