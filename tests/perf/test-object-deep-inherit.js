// When an object is garbage collected by Duktape, a finalizer
// lookup is performed.  This lookup includes inheritance, so
// for very deep structures there are O(N^2) own property lookups
// as each level of the deep hierarchy is finalized.

if (typeof print !== 'function') { print = console.log; }

function mkObj(n) {
    var res = { foo: 123 };
    while (--n > 0) {
        res = Object.create(res);
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
