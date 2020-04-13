/*
 *  Basic function call performance.
 */

if (typeof print !== 'function') { print = console.log; }

function func(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9) {
    // allocate regs so that meaningful value stack resizes happen
    var x00, x01, x02, x03, x04, x05, x06, x07, x08, x09;
    var x10, x11, x12, x13, x14, x15, x16, x17, x18, x19;
    var x20, x21, x22, x23, x24, x25, x26, x27, x28, x29;
    var x30, x31, x32, x33, x34, x35, x36, x37, x38, x39;
    var x40, x41, x42, x43, x44, x45, x46, x47, x48, x49;
    var x50, x51, x52, x53, x54, x55, x56, x57, x58, x59;
    var x60, x61, x62, x63, x64, x65, x66, x67, x68, x69;
    var x70, x71, x72, x73, x74, x75, x76, x77, x78, x79;
    var x80, x81, x82, x83, x84, x85, x86, x87, x88, x89;
    var x90, x91, x92, x93, x94, x95, x96, x97, x98, x99;
    x99 = 1;

    return;
}

function test() {
    var i;
    var f = func;  // eliminate slow path lookup from results

    var t1 = Date.now();

    for (i = 0; i < 1e7; i++) {
        f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    }

    var t2 = Date.now();
    print((1e7 * 10 / (t2 - t1)) + ' calls per millisecond');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
