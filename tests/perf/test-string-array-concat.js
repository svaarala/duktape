/*
 *  Build string with plain concat.  Causes O(n^2) behavior because every
 *  intermediate step is string interned.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i, j;
    var t;

    for (i = 0; i < 5e3; i++) {
        t = [];
        for (j = 0; j < 1e4; j++) {
            t[j] = 'x';
        }
        t = t.join('');
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
