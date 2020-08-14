// Like test-object-deep-inherit.js but the object is deep via ordinary
// references (not [[Prototype]]) which has no O(N^2) effect for
// _Finalizer lookup.

if (typeof print !== 'function') { print = console.log; }

function mkObj(n) {
    var res = { foo: 123 };
    while (--n > 0) {
        res = { ref: res };
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
