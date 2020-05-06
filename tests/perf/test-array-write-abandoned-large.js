/*
 *  Basic array write performance for abandoned arrays
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [];
    var i;

    for (i = 0; i < 1e5; i++) {
        arr[i] = i;
    }

    arr[1e7] = 1;
    arr.length = 1e5;
    //print(JSON.stringify(Duktape.info(arr)));

    for (i = 0; i < 1e7; i++) {
        arr[1000] = 234;
        arr[2001] = 234;
        arr[3002] = 234;
        arr[4003] = 234;
        arr[5004] = 234;
        arr[6005] = 234;
        arr[7006] = 234;
        arr[8007] = 234;
        arr[9008] = 234;
        arr[10009] = 234;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
