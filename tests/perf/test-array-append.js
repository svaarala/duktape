/*
 *  Basic array append-by-index performance
 */

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

test();
