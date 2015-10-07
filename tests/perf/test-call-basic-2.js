/*
 *  Basic function call performance.
 */

if (typeof print !== 'function') { print = console.log; }

function func() {
    return;
}

function test() {
    var i;
    var f = func;  // eliminate slow path lookup from results

    for (i = 0; i < 1e7; i++) {
        f();
        f();
        f();
        f();
        f();
        f();
        f();
        f();
        f();
        f();
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
