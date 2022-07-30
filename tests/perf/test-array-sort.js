/*
 *  Array sort() test
 */

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

test();
