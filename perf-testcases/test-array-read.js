/*
 *  Basic array read performance
 */

function test() {
    var arr = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
    var i;
    var ign;

    for (i = 0; i < 1e8; i++) {
        ign = arr[7];
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
