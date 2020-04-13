/*
 *  Basic array .length write performance
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [];
    var i, j;

    for (i = 0; i < 3e7; i++) {
        arr.length = 1;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
