/*
 *  Making a copy of a plain buffer with Ecmascript code (example in guide).
 */

/*---
{
    "custom": true
}
---*/

/*===
buffer ABCD
buffer ABCD
false
buf: ABCD
copy: XBCD
===*/

function bufferCopyTest() {
    var buf = Duktape.dec('hex', '41424344');  // ABCD
    print(typeof buf, buf);

    // Create a Duktape.Buffer object which is accepted by Uint8Array
    // constructor.  The constructor creates a new buffer and copies
    // the input bytes into the new buffer.  Finally, Duktape.Buffer
    // extracts the underlying copied buffer.
    //
    // (Right now Uint8Array() doesn't accept a plain buffer value
    // but that might change in later versions.)

    var copy = Duktape.Buffer(new Uint8Array(new Duktape.Buffer(buf)));
    print(typeof copy, copy);
    print(copy === buf);

    // Demonstrate independence
    copy[0] = ('X').charCodeAt(0);
    print('buf:', buf);
    print('copy:', copy);
}

try {
    bufferCopyTest();
} catch (e) {
    print(e.stack || e);
}
