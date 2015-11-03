/*
 *  Loading property read to register.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var t;
    var obj = { foo: 123 };

    for (i = 0; i < 1e6; i++) {
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;

        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;

        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;

        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;

        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;

        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;

        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;

        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;

        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;

        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
        t = obj.foo;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
