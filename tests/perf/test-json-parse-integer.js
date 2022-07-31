/*
 *  Test JSON number parsing (decode loop) for fast path integers.
 */

function test() {
    var arr = [];
    var i;

    for (i = 0; i < 1e4; i++) {
        arr[i] = Math.floor(Math.random() * 1e9);
    }

    var jsondata = JSON.stringify(arr);
    print(jsondata.length);
    //print(jsondata);

    for (i = 0; i < 1000; i++) {
        void JSON.parse(jsondata);
    }
}

test();
