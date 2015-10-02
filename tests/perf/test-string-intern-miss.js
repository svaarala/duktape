/*
 *  Test creation and interning of new strings when the intern check
 *  misses any existing strings in the string table.  Exercises string
 *  hashing and memory churn.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var buf = Duktape.Buffer(2048);
    var i;

    for (i = 0; i < buf.length; i++) {
        buf[i] = i;
    }

    for (i = 0; i < 1e6; i++) {
        // make buffer value unique
        buf[0] = i;
        buf[1] = i >> 8;
        buf[2] = i >> 16;

        void String(buf);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
