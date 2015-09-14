/*
 *  Basic function call performance.
 */

function test() {
    var i;

    function f() { return; }

    for (i = 0; i < 1e8; i++) {
        f();
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
