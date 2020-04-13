/*
 *  Basic array read performance for abandoned arrays
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [];
    var i;
    var ign;

    for (i = 0; i < 1e5; i++) {
        arr[i] = i;
    }

    arr[1e7] = 1;
    arr.length = 1e5;
    //print(JSON.stringify(Duktape.info(arr)));

    for (i = 0; i < 1e7; i++) {
        ign = arr[1000];
        ign = arr[2001];
        ign = arr[3002];
        ign = arr[4003];
        ign = arr[5004];
        ign = arr[6005];
        ign = arr[7006];
        ign = arr[8007];
        ign = arr[9008];
        ign = arr[10009];
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
