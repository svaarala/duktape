/*
 *  Basic property write performance
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var obj = { xxx1: 1, xxx2: 2, xxx3: 3, xxx4: 4, foo: 123 };
    var i;

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
