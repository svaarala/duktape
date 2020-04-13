/*
 *  Reading a variable.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var t;

    try {
        throw new TypeError('dummy');
    } catch (e) {
        for (i = 0; i < 1e6; i++) {
            t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e;
            t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e;
            t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e;
            t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e;
            t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e;
            t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e;
            t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e;
            t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e;
            t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e;
            t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e; t = e;
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
