/*
 *  Test creation and interning of new strings when the intern check
 *  matches an existing string in the string table.  Exercises string
 *  hashing but limited memory churn.
 */

function test() {
    var buf = Duktape.Buffer(2048);
    var i;

    for (i = 0; i < buf.length; i++) {
        buf[i] = i;
    }

    for (i = 0; i < 1e6; i++) {
        void String(buf);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
