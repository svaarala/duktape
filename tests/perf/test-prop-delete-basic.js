/*
 *  Basic property delete performance
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var obj = {};
    var i;
    var ign;

    for (i = 0; i < 1024 - 1; i++) {
        obj['prop-' + i] = 1;
    }
    obj['foo'] = 123;

    // This in practice only tests deletion of a non-existent
    // property because the property is deleted once and not
    // added back.
    for (i = 0; i < 1e7; i++) {
        ign = delete obj.foo;
        ign = delete obj.foo;
        ign = delete obj.foo;
        ign = delete obj.foo;
        ign = delete obj.foo;
        ign = delete obj.foo;
        ign = delete obj.foo;
        ign = delete obj.foo;
        ign = delete obj.foo;
        ign = delete obj.foo;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
