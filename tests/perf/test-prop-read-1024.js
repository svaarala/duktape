/*
 *  Basic property read performance
 */

// XXX: Perf tests should access all or many of the properties to get a
// better sense of the average case.  Tests should also cover cases where
// an object has tens of millions of properties which are accessed in a
// pseudorandom sequence (computed beforehand and reused?) which has
// different cache behavior

if (typeof print !== 'function') { print = console.log; }

function test() {
    var obj = {};
    var i;
    var ign;

    for (i = 0; i < 1024 - 1; i++) {
        obj['prop-' + i] = 1;
    }
    obj['foo'] = 123;
    if (typeof Duktape !== 'undefined') { Duktape.compact(obj); }

    for (i = 0; i < 1e7; i++) {
        ign = obj.foo;
        ign = obj.foo;
        ign = obj.foo;
        ign = obj.foo;
        ign = obj.foo;
        ign = obj.foo;
        ign = obj.foo;
        ign = obj.foo;
        ign = obj.foo;
        ign = obj.foo;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
