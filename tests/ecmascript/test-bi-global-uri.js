/*
 *  Tests for global object URI handling functions:
 *
 *    - encodeURI()
 *    - encodeURIComponent()
 *    - decodeURI()
 *    - decodeURIComponent()
 */

// indirect eval -> this is bound to the global object, E5 Section 10.4.2, step 1.a.
var g = (function () { var e = eval; return e('this'); } )();

/* Pure Ecmascript helper to URI encode a codepoint into URI escaped form.
 * Allows surrogate pairs to be encoded into invalid UTF-8 on purpose.
 */
function encCodePoint(x, forced_len) {
    var len;
    var initial;
    var i;
    var nybbles = "0123456789ABCDEF";
    var initial_bytes = [ null, 0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe ];
    var t;
    var res;

    // Supports extended UTF-8 up to 36 bits
    if (x < 0x80) { len = 1; }
    else if (x < 0x800) { len = 2; }
    else if (x < 0x10000) { len = 3; }
    else if (x < 0x200000) { len = 4; }
    else if (x < 0x4000000) { len = 5; }
    else if (x < 0x80000000) { len = 6; }
    else { len = 7; }

    if (typeof forced_len === 'number') {
        len = forced_len;
    }
    initial = initial_bytes[len];

    t = [];
    for (i = len - 1; i >= 0; i--) {
        if (i === 0) {
            t[i] = initial + x;
        } else {
            t[i] = (x & 0x3f) + 0x80;
        }
        x = x >>> 6;
    }

    res = [];
    for (i = 0; i < len; i++) {
        res.push('%' + nybbles.charAt((t[i] >>> 4) & 0x0f) +
                 nybbles.charAt(t[i] & 0x0f));
    }

    return res.join('');
}

/* Dump a string as decimal codepoints, ensures that tests produce ASCII only
 * outputs.
 */
function dumpCodePoints(x) {
    var i;
    var res = [];

    for (i = 0; i < x.length; i++) {
        res.push(x.charCodeAt(i));
    }

    return res.join(' ');
}

/*===
basic encode
http://www.example.com/%C3%8A%D8%80%E1%88%B4#foo
104 116 116 112 58 47 47 119 119 119 46 101 120 97 109 112 108 101 46 99 111 109 47 202 1536 4660 35 102 111 111
http%3A%2F%2Fwww.example.com%2F%C3%8A%D8%80%E1%88%B4%23foo
104 116 116 112 58 47 47 119 119 119 46 101 120 97 109 112 108 101 46 99 111 109 47 202 1536 4660 35 102 111 111
===*/

/* A simple URI encoding / decoding test.
 *
 * Note: upper case hex escapes are required by the encoding
 * algorithm in E5.1 Section 15.1.3.
 */

print('basic encode');

function basicEncodeTest() {
    var uri = 'http://www.example.com/\u00ca\u0600\u1234#foo';
    var t;

    t = g.encodeURI(uri);
    print(t);
    t = g.decodeURI(t);
    print(dumpCodePoints(t));

    t = g.encodeURIComponent(uri);
    print(t);
    t = decodeURIComponent(t);
    print(dumpCodePoints(t));
}

try {
    basicEncodeTest();
} catch (e) {
    print(e.name);
}

/*===
encoding of ascii range
%00%01%02%03%04%05%06%07%08%09%0A%0B%0C%0D%0E%0F%10%11%12%13%14%15%16%17%18%19%1A%1B%1C%1D%1E%1F%20!%22#$%25&'()*+,-./0123456789:;%3C=%3E?@ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E_%60abcdefghijklmnopqrstuvwxyz%7B%7C%7D~%7F
0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127
%00%01%02%03%04%05%06%07%08%09%0A%0B%0C%0D%0E%0F%10%11%12%13%14%15%16%17%18%19%1A%1B%1C%1D%1E%1F%20!%22%23%24%25%26'()*%2B%2C-.%2F0123456789%3A%3B%3C%3D%3E%3F%40ABCDEFGHIJKLMNOPQRSTUVWXYZ%5B%5C%5D%5E_%60abcdefghijklmnopqrstuvwxyz%7B%7C%7D~%7F
0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127
===*/

/* ASCII range test for character encoding */

print('encoding of ascii range');

function asciiEncodeTest() {
    var i;
    var txt = [];
    var t;

    for (i = 0; i < 128; i++) {
        txt.push(String.fromCharCode(i));
    }
    txt = txt.join('');

    t = g.encodeURI(txt);
    print(t);
    t = g.decodeURI(t);
    print(dumpCodePoints(t));

    t = g.encodeURIComponent(txt);
    print(t);
    t = decodeURIComponent(t);
    print(dumpCodePoints(t));
}

try {
    asciiEncodeTest();
} catch (e) {
    print(e.name);
}

/*===
decoding of ascii range
0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 37 50 51 37 50 52 37 37 50 54 39 40 41 42 37 50 66 37 50 67 45 46 37 50 70 48 49 50 51 52 53 54 55 56 57 37 51 65 37 51 66 60 37 51 68 62 37 51 70 37 52 48 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127
0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127
===*/

print('decoding of ascii range');

function asciiDecodeTest() {
    var i;
    var txt = [];
    var res;

    for (i = 0; i < 128; i++) {
        txt.push(encCodePoint(i));
    }
    txt = txt.join('');

    // since e.g. control characters will be output, dump as codepoints
    print(dumpCodePoints(g.decodeURI(txt)));
    print(dumpCodePoints(g.decodeURIComponent(txt)));
}

try {
    asciiDecodeTest();
} catch (e) {
    print(e.name);
}

/*===
decode non-bmp
55296 56320
55296 56320
55304 57157
55304 57157
55441 56679
55441 56679
55804 56338
55804 56338
56256 56320
56256 56320
56260 56884
56260 56884
56319 57343
56319 57343
===*/

/* Decode non-BMP characters and check that surrogate pairs are decoded
 * correctly.  In other words, a single UTF-8 encoded codepoint becomes
 * two Ecmascript codepoints.
 *
 * Decoding of non-BMP characters above U+10FFFF is required to result
 * in URIError, and is tested separately below in invalid UTF-8 tests.
 */

print('decode non-bmp');

function decodeNonBmpTest() {
    var inputs = [
        encCodePoint(0x10000),
        encCodePoint(0x12345),
        encCodePoint(0x34567),
        encCodePoint(0x8f012),
        encCodePoint(0x100000),
        encCodePoint(0x101234),
        encCodePoint(0x10ffff)
    ];
    var i;

    for (i = 0; i < inputs.length; i++) {
        print(dumpCodePoints(g.decodeURI(inputs[i])));
        print(dumpCodePoints(g.decodeURIComponent(inputs[i])));
    }
}

try {
    decodeNonBmpTest();
} catch (e) {
    print(e.name);
}

/*===
combine surrogate pairs in encode
%F0%90%80%80
%F0%90%8F%BF
%F0%90%90%80
%F4%8F%B0%80
%F4%8F%BF%BF
===*/

/* When encoding, surrogate pairs found in Ecmascript strings must be combined,
 * and encoded into UTF-8 (as a single codepoint).
 */

print('combine surrogate pairs in encode');

try {
    print(g.encodeURI('\ud800\udc00'));
    print(g.encodeURI('\ud800\udfff'));
    print(g.encodeURI('\ud801\udc00'));
    print(g.encodeURI('\udbff\udc00'));
    print(g.encodeURI('\udbff\udfff'));
} catch (e) {
    print(e.name);
}

/*===
attempt to encode invalid surrogate pairs
%ED%9F%BF
URIError
URIError
URIError
URIError
%EE%80%80
URIError
URIError
URIError
%F0%90%80%80
%F0%90%8F%BF
URIError
URIError
URIError
URIError
URIError
URIError
URIError
===*/

/* An attempt to encode an invalid surrogate pair is a URIError. */

print('attempt to encode invalid surrogate pairs');

function attemptInvalidSurrogateEncode(x) {
    try {
        print(g.encodeURI(x));
    } catch (e) {
        print(e.name);
    }
}

try {
    attemptInvalidSurrogateEncode('\ud7ff');  // ok
    attemptInvalidSurrogateEncode('\ud800');
    attemptInvalidSurrogateEncode('\udbff');
    attemptInvalidSurrogateEncode('\udc00');
    attemptInvalidSurrogateEncode('\udfff');
    attemptInvalidSurrogateEncode('\ue000');  // ok

    attemptInvalidSurrogateEncode('\ud800\ud7ff');
    attemptInvalidSurrogateEncode('\ud800\ud800');
    attemptInvalidSurrogateEncode('\ud800\udbff');
    attemptInvalidSurrogateEncode('\ud800\udc00');  // ok
    attemptInvalidSurrogateEncode('\ud800\udfff');  // ok
    attemptInvalidSurrogateEncode('\ud800\ue000');

    attemptInvalidSurrogateEncode('\udc00\ud7ff');
    attemptInvalidSurrogateEncode('\udc00\ud800');
    attemptInvalidSurrogateEncode('\udc00\udbff');
    attemptInvalidSurrogateEncode('\udc00\udc00');
    attemptInvalidSurrogateEncode('\udc00\udfff');
    attemptInvalidSurrogateEncode('\udc00\ue000');
} catch (e) {
    print(e.name);
}

/*===
invalid utf-8 decode
%C0%80
%E0%80%A9
%F0%80%80%A9
%F8%80%80%80%A9
%FC%80%80%80%80%A9
%FE%80%80%80%80%80%A9
56319 57343
56319 57343
===*/

/* Decode only allows valid UTF-8 encodings up to 4 bytes.  This technically
 * allows codepoints up to U+1FFFFF, but UTF-8 further restricts the range to
 * U+10FFFF.  Codepoints above U+10FFFF would not fit into surrogate pairs
 * anyway.
 *
 * Surrogate pair codepoints (U+D800...U+DFFF) encoded into UTF-8 naively
 * are not allowed by UTF-8.  Non-shortest encodings are not allowed by
 * UTF-8 either.
 *
 * Things to test:
 *
 *   - Surrogate pairs naively encoded into UTF-8 (= CESU-8) cause an URIError
 *
 *   - Non-shortest UTF-8 encodings, e.g. URIError is required for C0 80.
 *
 *   - U+10FFFF decodes correctly to an Ecmascript string with a surrogate pair
 *
 *   - U+110000 causes an URIError
 *
 *   - Codepoints with >4 byte encoding cause an URIError.
 *
 * From RFC 3629:
 *
 *   Implementations of the decoding algorithm above MUST protect against
 *   decoding invalid sequences.  For instance, a naive implementation may
 *   decode the overlong UTF-8 sequence C0 80 into the character U+0000,
 *   or the surrogate pair ED A1 8C ED BE B4 into U+233B4.  Decoding
 *   invalid sequences may have security consequences or cause other
 *   problems.
 */

print('invalid utf-8 decode');

var _invalidInputErrorReported = false;

function testInvalidUtf8Input(x) {
    var ok1, ok2;

    try {
        g.decodeURI(x);
        ok1 = true;
    } catch (e) {
        if (e.name === 'URIError') {
            ok1 = false;
        } else {
            throw e;
        }
    }

    try {
        g.decodeURIComponent(x);
        ok2 = true;
    } catch (e) {
        if (e.name === 'URIError') {
            ok2 = false;
        } else {
            throw e;
        }
    }

    if (!ok1 && !ok2) {
        // silent
    } else {
        // one or both inputs did NOT produce a URIError; only report first
        // error because Rhino causes a flood otherwise

        if (!_invalidInputErrorReported) {
            _invalidInputErrorReported = true;
            print('first error', x, ok1, ok2);
        }
    }
}

function testValidUtf8Input(x) {
    var t;

    try {
        t = g.decodeURI(x);
        print(dumpCodePoints(t));
    } catch (e) {
        print(e.name, '(unexpected)');
    }

    try {
        t = g.decodeURIComponent(x);
        print(dumpCodePoints(t));
    } catch (e) {
        print(e.name, '(unexpected)');
    }
}

function invalidUtf8Test() {
    var i;
    var t;

    // surrogate pairs
    for (i = 0xd800; i < 0xe000; i++) {
        testInvalidUtf8Input(encCodePoint(i));
    }

    // even valid surrogate pairs (or any surrogate character pairs)
    for (i = 0xd800; i < 0xe000; i++) {
        // we're just spot checking the second codepoint to keep the runtime reasonable
        for (j = 0xd800; j < 0xe000; j += 127) {
            testInvalidUtf8Input(encCodePoint(i) + encCodePoint(j));
        }
    }

    // non-shortest encodings (C0 80 mentioned explicitly in spec);
    // above 4 bytes rejected because not valid UTF-8 (in addition
    // to not being shortest)
    t = encCodePoint(0, 2); print(t);
    testInvalidUtf8Input(t);
    t = encCodePoint(41, 3); print(t);
    testInvalidUtf8Input(t);
    t = encCodePoint(41, 4); print(t);
    testInvalidUtf8Input(t);
    t = encCodePoint(41, 5); print(t);
    testInvalidUtf8Input(t);
    t = encCodePoint(41, 6); print(t);
    testInvalidUtf8Input(t);
    t = encCodePoint(41, 7); print(t);
    testInvalidUtf8Input(t);

    // U+10FFFF decodes correctly
    testValidUtf8Input('%F4%8F%BF%BF');

    // codepoints above utf-8 range
    testInvalidUtf8Input(encCodePoint(0x110000));
    testInvalidUtf8Input(encCodePoint(0x200123));
    testInvalidUtf8Input(encCodePoint(0x4001234));
    testInvalidUtf8Input(encCodePoint(0x80012345));
    testInvalidUtf8Input(encCodePoint(0xfedcba98));

    // invalid surrogate pair EDA18C EDBEB4 (from RFC 3629)
    // EDA18C -> U+D84C  (invalid UTF-8)
    // EDBEB4 -> U+DFB4  (invalid UTF-8)
    testInvalidUtf8Input('%ED%A1%8C%ED%BE%B4');

    // invalid UTF-8 bytes; C2 must be followed by a byte >= 0x80
    testInvalidUtf8Input('%C2%01');
}

try {
    invalidUtf8Test();
} catch (e) {
    print(e.name);
}

/*===
broken escapes
URIError
URIError
URIError
URIError
65
65
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
URIError
56319 57343
56319 57343
===*/

/* Test truncated hex encoding (e.g. '%' or '%1') and truncated UTF-8 encoding
 * (e.g. '%C2' but no followup byte).
 */

print('broken escapes');

function testBrokenEscapes() {
    var inputs = [
        '%',
        '%4',
        '%41',

        // partial encodings of U+10FFFF
        '%',
        '%F',
        '%F4',
        '%F4%',
        '%F4%8',
        '%F4%8F',
        '%F4%8F%',
        '%F4%8F%B',
        '%F4%8F%BF',
        '%F4%8F%BF%',
        '%F4%8F%BF%B',
        '%F4%8F%BF%BF',
    ];
    var i;

    for (i = 0; i < inputs.length; i++) {
        try {
            print(dumpCodePoints(g.decodeURI(inputs[i])));
        } catch (e) {
            print(e.name);
        }
        try {
            print(dumpCodePoints(g.decodeURIComponent(inputs[i])));
        } catch (e) {
            print(e.name);
        }
    }
}

try {
    testBrokenEscapes();
} catch (e) {
    print(e.name);
}
