/*
 *  Basic function call performance.
 */

if (typeof print !== 'function') { print = console.log; }

function func() {
    // allocate regs so that meaningful value stack resizes happen
    var x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;
    var y0, y1, y2, y3, y4, y5, y6, y7, y8, y9;
    var z0, z1, z2, z3, z4, z5, z6, z7, z8, z9;
    var z9 = 1;

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
