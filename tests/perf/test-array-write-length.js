/*
 *  Basic array .length write performance
 */

function test() {
    var arr = [];
    var i, j;

    for (i = 0; i < 3e7; i++) {
        arr.length = 1;
    }
}

test();
