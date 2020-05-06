/*
 *  Basic array read performance
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
    var i;
    var ign;

    for (i = 0; i < 1e7; i++) {
        ign = arr[0];
        ign = arr[1];
        ign = arr[2];
        ign = arr[3];
        ign = arr[4];
        ign = arr[5];
        ign = arr[6];
        ign = arr[7];
        ign = arr[8];
        ign = arr[9];
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
