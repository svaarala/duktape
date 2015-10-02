/*
 *  Test addition when it involves a NaN, which matters on x86.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var x = 0 / 0;
    var y = 234.5;  // not fastint
    var t;

    for (i = 0; i < 1e6; i++) {
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
