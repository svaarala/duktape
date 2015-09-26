/*
 *  Test JSON number parsing (decode loop) for arbitrary numbers.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [];
    var i;

    for (i = 0; i < 1e4; i++) {
        arr[i] = Math.random() * 1e9;
    }

    var jsondata = JSON.stringify(arr);
    print(jsondata.length);
    //print(jsondata);

    for (i = 0; i < 100; i++) {
        void JSON.parse(jsondata);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
