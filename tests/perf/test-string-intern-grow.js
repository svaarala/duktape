/*
 *  Test creation and interning of new strings when the intern check
 *  misses any existing strings in the string table.  Grow the string
 *  table a lot and then repeat.  Exercises string hashing, string table
 *  performance, and memory churn.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var buf = Duktape.Buffer(2048);
    var i, j;
    var arr;

    for (i = 0; i < buf.length; i++) {
        buf[i] = i;
    }

    for (i = 0; i < 1e3; i++) {
        arr = [];
        for (j = 0; j < 1e3; j++) {
            // make buffer value unique
            buf[0] = j;
            buf[1] = j >> 8;
            buf[2] = j >> 16;
            arr[j] = "" + buf;
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
