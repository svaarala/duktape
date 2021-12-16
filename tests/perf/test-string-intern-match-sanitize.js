/*
 *  Test creation and interning of new strings when the intern check
 *  matches an existing string in the string table.  Exercises string
 *  hashing but limited memory churn.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var buf = (Uint8Array.allocPlain || Duktape.Buffer)(2048);
    var ref;
    var i;
    var bufferToString = String.fromBufferRaw || String;

    for (i = 0; i < buf.length; i++) {
        buf[i] = i;
    }
    ref = bufferToString(buf);

    for (i = 0; i < 1e6; i++) {
        void bufferToString(buf);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
