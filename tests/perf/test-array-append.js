/*
 *  Basic array append-by-index performance
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr;
    var i, j;

    for (i = 0; i < 1e3; i++) {
        arr = [];
        for (j = 0; j < 1e4; j++) {
            arr[j] = 'foo';
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
