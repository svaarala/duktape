/*
 *  Basic property write performance, array index writes for existing indices
 */

function test() {
    var arr = [];
    var i;

    for (i = 0; i < 1e7; i++) {
        arr[0] = 123;
        arr[1] = 123;
        arr[2] = 123;
        arr[3] = 123;
        arr[4] = 123;
        arr[5] = 123;
        arr[6] = 123;
        arr[7] = 123;
        arr[8] = 123;
        arr[9] = 123;
    }
}

test();
