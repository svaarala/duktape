/*
 *  Basic property read performance for an inherited property
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var parent = { xxx1: 1, xxx2: 2, xxx3: 3, xxx4: 4, foo: 123 };
    var i;
    var ign;
    var obj = Object.create(parent);
    obj = Object.create(obj);  // two levels of inheritance

    for (i = 0; i < 1e7; i++) {
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
