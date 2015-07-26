/*
 *  Basic array write performance
 */

function test() {
    var arr = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
    var i;

    for (i = 0; i < 1e8; i++) {
        arr[7] = 234;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
