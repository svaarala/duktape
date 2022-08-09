/*@include util-buffer.js@*/
/*@include util-string.js@*/

/*---
custom: true
---*/

/*===
"{string:"A"}"
"{"string":"A"}"
"{string:"\u0324"}"
"{"string":"<U+0324>"}"
"{string:"\ufedc"}"
"{"string":"<U+FEDC>"}"
"{string:"\U0010ffff"}"
"{"string":"<U+DBFF><U+DFFF>"}"
"{string:"\ufffd\ufffd\ufffd\ufffd"}"
"{"string":"<U+FFFD><U+FFFD><U+FFFD><U+FFFD>"}"
"{string:"\ufffd\ufffd\ufffd\ufffd"}"
"{"string":"<U+FFFD><U+FFFD><U+FFFD><U+FFFD>"}"
"{string:"\ufffd\ufffd\ufffd\ufffd\ufffd"}"
"{"string":"<U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD>"}"
"{string:"\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd"}"
"{"string":"<U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD>"}"
"{string:"\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd"}"
"{"string":"<U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD>"}"
"{string:"\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd"}"
"{"string":"<U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD>"}"
"{string:"\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd"}"
"{"string":"<U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD>"}"
===*/

/* Test encoding of non-BMP characters. */

function nonBmpTest() {
    var t;
    var i;
    var buf;

    // http://en.wikipedia.org/wiki/UTF-8#Description
    // http://en.wikipedia.org/wiki/UTF-8#Extending_from_31_bit_to_36_bit_range
    var buffers = [
        Duktape.dec('hex', '41'),              // U+0041 (1 byte)
        Duktape.dec('hex', 'cca4'),            // U+0324 (2 bytes)
        Duktape.dec('hex', 'efbb9c'),          // U+FEDC (3 bytes)
        Duktape.dec('hex', 'f48fbfbf'),        // U+0010FFFF (4 bytes)
        Duktape.dec('hex', 'f490a0a0'),        // U+00110000 (4 bytes, above standard UTF-8)
        Duktape.dec('hex', 'f7beb78b'),        // U+001FEDCB (4 bytes, above standard UTF-8)
        Duktape.dec('hex', 'fbbfadb2ba'),      // U+03FEDCBA (5 bytes)
        Duktape.dec('hex', 'fdbfbb9caea9'),    // U+7FEDCBA9 (6 bytes)
        Duktape.dec('hex', 'fe83beb78baa98'),  // U+FEDCBA98 (7 bytes)
        Duktape.dec('hex', 'fe83bfbfbfbfbf'),  // U+FFFFFFFF (7 bytes)
        Duktape.dec('hex', 'febfbfbfbfbfbf'),  // U+FFFFFFFFF (7 bytes, max for 7-byte encoding)
    ];

    for (i = 0; i < buffers.length; i++) {
        buf = buffers[i];

        t = Duktape.enc('jx', { string: bufferToStringRaw(buf) });
        safePrintString(t);
        t = JSON.stringify({ string: bufferToStringRaw(buf) });
        safePrintString(t);
    }
}

nonBmpTest();
