/*---
{
    "custom": true
}
---*/

/*===
{"string":"A"}
123 34 115 116 114 105 110 103 34 58 34 65 34 125
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 804 34 125
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 65244 34 125
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 1114111 34 125
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 2092491 34 125
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 67034298 34 125
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 2146290601 34 125
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 4275878552 34 125
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 4294967295 34 125
===*/

/* Test encoding of non-BMP characters. */

function codepointDump(t) {
    var i;
    var tmp = [];

    for (i = 0; i < t.length; i++) {
        tmp.push(t.charCodeAt(i));
    }

    print(tmp.join(' '));
}

function safePrint(t) {
    var tmp = [];
    var cp;

    for (i = 0; i < t.length; i++) {
        cp = t.charCodeAt(i);
        if (cp >= 0x20 && cp <= 0x7e) {
            tmp.push(t.charAt(i));
        } else {
            tmp.push('?');
        }
    }

    print(tmp.join(''));
}
 
function nonBmpTest() {
    var t;
    var i;
    var buf;

    // http://en.wikipedia.org/wiki/UTF-8#Description
    // http://en.wikipedia.org/wiki/UTF-8#Extending_from_31_bit_to_36_bit_range
    var buffers = [
        __duk__.dec('hex', '41'),              // U+0041 (1 byte)
        __duk__.dec('hex', 'cca4'),            // U+0324 (2 bytes)
        __duk__.dec('hex', 'efbb9c'),          // U+FEDC (3 bytes)
        __duk__.dec('hex', 'f48fbfbf'),        // U+0010FFFF (4 bytes)
        __duk__.dec('hex', 'f7beb78b'),        // U+001FEDCB (4 bytes, above standard UTF-8)
        __duk__.dec('hex', 'fbbfadb2ba'),      // U+3FEDCBA (5 bytes)
        __duk__.dec('hex', 'fdbfbb9caea9'),    // U+7FEDCBA9 (6 bytes)
        __duk__.dec('hex', 'fe83beb78baa98'),  // U+FEDCBA98 (7 bytes)
        __duk__.dec('hex', 'fe83bfbfbfbfbf'),  // U+FFFFFFFF (7 bytes, max value that we actually support)

        // This is skipped as we don't support 7-byte encodings above
        // unsigned 32 bit range.
        //__duk__.dec('hex', 'febfbfbfbfbfbf'),  // U+FFFFFFFFF (7 bytes, max for 7-byte encoding)
    ];

    for (i = 0; i < buffers.length; i++) {
        buf = buffers[i];

        t = JSON.stringify({ string: String(buf) });
        safePrint(t);
        codepointDump(t);
    }

    // FIXME: also test ASCII only encoding for these (\u and \U)
}

try {
    nonBmpTest();
} catch (e) {
    print(e.name);
}
