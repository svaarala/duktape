/*
 *  Basic tail call performance.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;

    function f(x) { if (x <= 1) { return; } else { return f(x - 1); } }

    var t1 = Date.now();

    for (i = 0; i < 1e5; i++) {
        f(100);
    }

    var t2 = Date.now();
    print((1e5 * 100 / (t2 - t1)) + ' calls per millisecond');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
