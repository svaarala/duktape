/*
 *  Array sort() test
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [];
    var i, j;
    var rnd = Math.random;

    for (i = 0; i < 5; i++) {
        for (j = 0; j < 10000; j++) {
            arr[j] = rnd();
        }

        arr.sort();
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
