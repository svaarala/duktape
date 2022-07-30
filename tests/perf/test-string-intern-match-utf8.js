/*
 *  Test creation and interning of new strings when the intern check
 *  matches an existing string in the string table.  Exercises string
 *  hashing but limited memory churn.
 */

function test() {
    var buf = (Uint8Array.allocPlain || Duktape.Buffer)(2048);
    var ref;
    var i;
    var bufferToString = String.fromBufferRaw || String;

    for (i = 0; i < buf.length; i++) {
        buf[i] = i & 0x7f;
    }
    buf[0] = 0xdf;  // U+07FF
    buf[1] = 0xbf;
    ref = bufferToString(buf);

    for (i = 0; i < 1e6; i++) {
        void bufferToString(buf);
    }
}

test();
