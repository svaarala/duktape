/*---
{
    "custom": true
}
---*/

/*===
{string:"A"}
{"string":"A"}
123 34 115 116 114 105 110 103 34 58 34 65 34 125
{string:"\u0324"}
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 804 34 125
{string:"\ufedc"}
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 65244 34 125
{string:"\U0010ffff"}
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 1114111 34 125
{string:"\U001fedcb"}
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 2092491 34 125
{string:"\U03fedcba"}
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 67034298 34 125
{string:"\U7fedcba9"}
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 2146290601 34 125
{string:"\Ufedcba98"}
{"string":"?"}
123 34 115 116 114 105 110 103 34 58 34 4275878552 34 125
{string:"\Uffffffff"}
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
        Duktape.dec('hex', '41'),              // U+0041 (1 byte)
        Duktape.dec('hex', 'cca4'),            // U+0324 (2 bytes)
        Duktape.dec('hex', 'efbb9c'),          // U+FEDC (3 bytes)
        Duktape.dec('hex', 'f48fbfbf'),        // U+0010FFFF (4 bytes)
        Duktape.dec('hex', 'f7beb78b'),        // U+001FEDCB (4 bytes, above standard UTF-8)
        Duktape.dec('hex', 'fbbfadb2ba'),      // U+03FEDCBA (5 bytes)
        Duktape.dec('hex', 'fdbfbb9caea9'),    // U+7FEDCBA9 (6 bytes)
        Duktape.dec('hex', 'fe83beb78baa98'),  // U+FEDCBA98 (7 bytes)
        Duktape.dec('hex', 'fe83bfbfbfbfbf'),  // U+FFFFFFFF (7 bytes, max value that we actually support)

        // This is skipped as we don't support 7-byte encodings above
        // unsigned 32 bit range.
        //Duktape.dec('hex', 'febfbfbfbfbfbf'),  // U+FFFFFFFFF (7 bytes, max for 7-byte encoding)
    ];

    for (i = 0; i < buffers.length; i++) {
        buf = buffers[i];

        t = Duktape.enc('jx', { string: String(buf) });
        safePrint(t);
        t = JSON.stringify({ string: String(buf) });
        safePrint(t);
        codepointDump(t);
    }
}

try {
    nonBmpTest();
} catch (e) {
    print(e.name);
}
