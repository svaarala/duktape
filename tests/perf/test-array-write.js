/*
 *  Basic array write performance
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
    var i;

    for (i = 0; i < 1e7; i++) {
        arr[0] = 234;
        arr[1] = 234;
        arr[2] = 234;
        arr[3] = 234;
        arr[4] = 234;
        arr[5] = 234;
        arr[6] = 234;
        arr[7] = 234;
        arr[8] = 234;
        arr[9] = 234;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
