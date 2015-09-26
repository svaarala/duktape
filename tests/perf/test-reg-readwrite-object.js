/*
 *  Test bytecode register assignments with object values.  Involves only
 *  a dispatch and a tval copy for a heap-allocated tval.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var a, b, c, d;
    var i;

    a = {}; b = {}; c = {}; d = {};

    for (i = 0; i < 1e6; i++) {
        // 100
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;

        // 100
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;

        // 100
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;

        // 100
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;

        // 100
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;

        // 100
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;

        // 100
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;

        // 100
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;

        // 100
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;

        // 100
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
        a = a; a = b; a = c; a = d; b = a; b = b; b = c; b = d; c = a; c = b;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
