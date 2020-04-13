/*
 *  Basic setter property write performance
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var obj = { xxx1: 1, xxx2: 2, xxx3: 3, xxx4: 4, set foo(v) { return; } };
    var i;
    if (typeof Duktape !== 'undefined') { Duktape.compact(obj); }

    for (i = 0; i < 1e6; i++) {
        obj.foo = 123;
        obj.foo = 123;
        obj.foo = 123;
        obj.foo = 123;
        obj.foo = 123;
        obj.foo = 123;
        obj.foo = 123;
        obj.foo = 123;
        obj.foo = 123;
        obj.foo = 123;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
