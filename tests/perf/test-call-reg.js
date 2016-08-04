/*
 *  Basic function call performance, call through a register.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;

    function f() { return; }

    var t1 = Date.now();

    for (i = 0; i < 4e5; i++) {
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
    }

    var t2 = Date.now();
    print((4e5 * 100 / (t2 - t1)) + ' calls per millisecond');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
