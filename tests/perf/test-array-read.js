/*
 *  Basic array read performance
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
    var i;
    var ign;

    for (i = 0; i < 1e7; i++) {
        ign = arr[7];
        ign = arr[7];
        ign = arr[7];
        ign = arr[7];
        ign = arr[7];
        ign = arr[7];
        ign = arr[7];
        ign = arr[7];
        ign = arr[7];
        ign = arr[7];
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
