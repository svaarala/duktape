/*
 *  Basic property write performance
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var obj = {};
    var i;
    var ign;

    for (i = 0; i < 16 - 1; i++) {
        obj['prop-' + i] = 1;
    }
    obj['foo'] = 123;
    if (typeof Duktape !== 'undefined') { Duktape.compact(obj); }

    for (i = 0; i < 1e7; i++) {
        obj.foo = 234;
        obj.foo = 234;
        obj.foo = 234;
        obj.foo = 234;
        obj.foo = 234;
        obj.foo = 234;
        obj.foo = 234;
        obj.foo = 234;
        obj.foo = 234;
        obj.foo = 234;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
