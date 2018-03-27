/*
 *  Node.js Buffer toString()
 *
 *  In Duktape 2.x Node.js Buffer.prototype.toString() does an actual UTF-8
 *  decoding.
 */

/*@include util-buffer.js@*/
/*@include util-string.js@*/

function makeBuffer(bytes) {
    var buf = new Buffer(bytes.length);
    for (var i = 0; i < bytes.length; i++) {
        buf[i] = (typeof bytes[i] === 'number' ? bytes[i] : bytes[i].charCodeAt(0));
    }
    return buf;
}

/*===
node.js Buffer toString() test
function
false
true
"ABC"
"ABC"
"ABC"
"DEFG"
"EFG"
"E"
"<U+CAFE>A"
"<U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD>"
"<U+FFFD>ABC"
"<U+FFFD><U+FFFD>B<U+FFFD>"
"DEFG"
NONE NONE "DEFG"
NONE undefined "DEFG"
NONE null "DEFG"
NONE true "DEFG"
NONE false "DEFG"
NONE [object Object] "DEFG"
NONE [object Object] "DEFG"
NONE -1 "DEFG"
NONE 0 "DEFG"
NONE 1 "DEFG"
NONE 2 "DEFG"
NONE 3 "DEFG"
NONE 4 "DEFG"
NONE 5 "DEFG"
undefined NONE "DEFG"
undefined undefined "DEFG"
undefined null ""
undefined true "D"
undefined false ""
undefined [object Object] "D"
undefined [object Object] "DEF"
undefined -1 ""
undefined 0 ""
undefined 1 "D"
undefined 2 "DE"
undefined 3 "DEF"
undefined 4 "DEFG"
undefined 5 "DEFG"
null NONE "DEFG"
null undefined "DEFG"
null null ""
null true "D"
null false ""
null [object Object] "D"
null [object Object] "DEF"
null -1 ""
null 0 ""
null 1 "D"
null 2 "DE"
null 3 "DEF"
null 4 "DEFG"
null 5 "DEFG"
true NONE "EFG"
true undefined "EFG"
true null ""
true true ""
true false ""
true [object Object] ""
true [object Object] "EF"
true -1 ""
true 0 ""
true 1 ""
true 2 "E"
true 3 "EF"
true 4 "EFG"
true 5 "EFG"
false NONE "DEFG"
false undefined "DEFG"
false null ""
false true "D"
false false ""
false [object Object] "D"
false [object Object] "DEF"
false -1 ""
false 0 ""
false 1 "D"
false 2 "DE"
false 3 "DEF"
false 4 "DEFG"
false 5 "DEFG"
[object Object] NONE "EFG"
[object Object] undefined "EFG"
[object Object] null ""
[object Object] true ""
[object Object] false ""
[object Object] [object Object] ""
[object Object] [object Object] "EF"
[object Object] -1 ""
[object Object] 0 ""
[object Object] 1 ""
[object Object] 2 "E"
[object Object] 3 "EF"
[object Object] 4 "EFG"
[object Object] 5 "EFG"
[object Object] NONE "G"
[object Object] undefined "G"
[object Object] null ""
[object Object] true ""
[object Object] false ""
[object Object] [object Object] ""
[object Object] [object Object] ""
[object Object] -1 ""
[object Object] 0 ""
[object Object] 1 ""
[object Object] 2 ""
[object Object] 3 ""
[object Object] 4 "G"
[object Object] 5 "G"
-1 NONE "DEFG"
-1 undefined "DEFG"
-1 null ""
-1 true "D"
-1 false ""
-1 [object Object] "D"
-1 [object Object] "DEF"
-1 -1 ""
-1 0 ""
-1 1 "D"
-1 2 "DE"
-1 3 "DEF"
-1 4 "DEFG"
-1 5 "DEFG"
0 NONE "DEFG"
0 undefined "DEFG"
0 null ""
0 true "D"
0 false ""
0 [object Object] "D"
0 [object Object] "DEF"
0 -1 ""
0 0 ""
0 1 "D"
0 2 "DE"
0 3 "DEF"
0 4 "DEFG"
0 5 "DEFG"
1 NONE "EFG"
1 undefined "EFG"
1 null ""
1 true ""
1 false ""
1 [object Object] ""
1 [object Object] "EF"
1 -1 ""
1 0 ""
1 1 ""
1 2 "E"
1 3 "EF"
1 4 "EFG"
1 5 "EFG"
2 NONE "FG"
2 undefined "FG"
2 null ""
2 true ""
2 false ""
2 [object Object] ""
2 [object Object] "F"
2 -1 ""
2 0 ""
2 1 ""
2 2 ""
2 3 "F"
2 4 "FG"
2 5 "FG"
3 NONE "G"
3 undefined "G"
3 null ""
3 true ""
3 false ""
3 [object Object] ""
3 [object Object] ""
3 -1 ""
3 0 ""
3 1 ""
3 2 ""
3 3 ""
3 4 "G"
3 5 "G"
4 NONE ""
4 undefined ""
4 null ""
4 true ""
4 false ""
4 [object Object] ""
4 [object Object] ""
4 -1 ""
4 0 ""
4 1 ""
4 2 ""
4 3 ""
4 4 ""
4 5 ""
5 NONE ""
5 undefined ""
5 null ""
5 true ""
5 false ""
5 [object Object] ""
5 [object Object] ""
5 -1 ""
5 0 ""
5 1 ""
5 2 ""
5 3 ""
5 4 ""
5 5 ""
===*/

function nodejsBufferToStringTest() {
    var b, s;

    // Check inheritance; not inherited from Object.prototype
    print(typeof Buffer.prototype.toString);
    print(Buffer.prototype.toString === Object.prototype.valueOf);
    print(Buffer.prototype.hasOwnProperty('toString'));

    // buf.toString([encoding], [start], [end])

    // Without arguments encoding defaults to UTF-8 and the entire
    // buffer is converted to string.  At least undefined and null
    // are accepted as "not defined" for encoding.
    b = new Buffer('ABC');
    safePrintString(b.toString());
    safePrintString(b.toString(undefined));
    safePrintString(b.toString(null));

    // If the buffer is a slice of an underlying buffer, only that slice
    // is string converted.  Offsets are relative to the slice.
    b = new Buffer('ABCDEFGH');
    b = b.slice(3, 7);  // DEFG
    safePrintString(b.toString());
    safePrintString(b.toString(null, 1));
    safePrintString(b.toString(null, 1, 2));

    // When the buffer data is legal UTF-8 and the chosen encoding
    // is UTF-8 (default), Duktape internal representation is correct
    // as is.  Here the 4-byte data is U+CAFE U+0041.  (Since Duktape 2.x
    // there's an explicit UTF-8 decoding + CESU-8 encoding process.)
    b = new Buffer(4);
    b[0] = 0xec; b[1] = 0xab; b[2] = 0xbe; b[3] = 0x41;
    safePrintString(b.toString());

    // When the buffer data is not legal UTF-8 replacement characters
    // (U+FFFD) are emitted.  In this case the input is a CESU-8 encoded
    // surrogate pair which is entirely invalid UTF-8.  In this case one
    // replacement character gets emitted for each byte.
    b = new Buffer(6);
    b[0] = 0xed; b[1] = 0xa0; b[2] = 0x80;
    b[3] = 0xed; b[4] = 0xbf; b[5] = 0xbf;
    safePrintString(b.toString());

    // Here the buffer data is invalid UTF-8 and invalid CESU-8.
    // Node.js replaces the offending character (0xff) with U+FFFD
    // (replacement character).
    b = new Buffer(4);
    b[0] = 0xff; b[1] = 0x41; b[2] = 0x42; b[3] = 0x43;
    safePrintString(b.toString());

    // Invalid continuation characters.  Node.js seems to scan for
    // the next valid starting byte and each offending byte causes
    // a new U+FFFD to be emitted (here U+FFFD U+FFFD U+0042 U+FFFD).
    // While there are differences in replacement character handling,
    // here the result agrees between Node.js and Duktape.
    b = new Buffer(4);
    b[0] = 0xc1; b[1] = 0xc1; b[2] = 0x42; b[3] = 0xc1;
    safePrintString(b.toString());

    // XXX: encoding test?

    // Offsets, very lenient, something like:
    //     - Non-numbers coerce to 0 (no valueOf() etc consulted)
    //     - Negative starting point => empty result, regardless of 'end'
    //     - Valid starting point but 'end' out of bounds => clamp to end
    //     - Crossed indices => empty result
    //
    // Duktape behavior is a bit more lenient: everything is ToInteger()
    // coerced, clamped to valid range, and crossed indices are allowed.
    // Testcase expect string has been corrected for this.
    //
    // Offsets are relative to a slice (if buffer is a slice).

    b = new Buffer('ABCDEFGH');
    b = b.slice(3, 7);  // DEFG
    safePrintString(b.toString());

    var offsetList = [
        'NONE',
        undefined,
        null,
        true,
        false,
        { valueOf: function () { return 1; } },
        { valueOf: function () { return 3; } },
        -1, 0, 1, 2, 3, 4, 5
    ]

    offsetList.forEach(function (start) {
        offsetList.forEach(function (end) {
            try {
                if (start === 'NONE') {
                    s = b.toString('utf8');
                } else if (end === 'NONE') {
                    s = b.toString('utf8', start);
                } else {
                    s = b.toString('utf8', start, end);
                }
                print(start, end, safeEscapeString(s));
            } catch (e) {
                print(start, end, e.name);
            }
        });
    });
}

try {
    print('node.js Buffer toString() test');
    nodejsBufferToStringTest();
} catch (e) {
    print(e.stack || e);
}

/*===
all bytes test
256
8396352
===*/

// Just a simple test which decodes a buffer containing all 256 byte values.

function allBytesTest() {
    var buf = new Buffer(256);
    var i;

    for (i = 0; i < 256; i++) {
        buf[i] = i;
    };

    var str = buf.toString();
    print(str.length);

    var cpsum = 0;
    for (i = 0; i < str.length; i++) {
        cpsum += str.charCodeAt(i);
    }
    print(cpsum);
}

try {
    print('all bytes test');
    allBytesTest();
} catch (e) {
    print(e.stack || e);
}

/*===
0 120
1 120
2 120
3 120
4 120
5 120
6 120
7 120
8 120
9 120
10 120
11 120
12 120
13 120
14 120
15 120
16 120
17 120
18 120
19 120
20 120
21 120
22 120
23 120
24 120
25 120
26 120
27 120
28 120
29 120
30 120
31 120
32 120
33 120
34 120
35 120
36 120
37 120
38 120
39 120
40 120
41 120
42 120
43 120
44 120
45 120
46 120
47 120
48 120
49 120
50 120
51 120
52 120
53 120
54 120
55 120
56 120
57 120
58 120
59 120
60 120
61 120
62 120
63 120
64 120
65 120
66 120
67 120
68 120
69 120
70 120
71 120
72 120
73 120
74 120
75 120
76 120
77 120
78 120
79 120
80 120
81 120
82 120
83 120
84 120
85 120
86 120
87 120
88 120
89 120
90 120
91 120
92 120
93 120
94 120
95 120
96 120
97 120
98 120
99 120
100 120
101 120
102 120
103 120
104 120
105 120
106 120
107 120
108 120
109 120
110 120
111 120
112 120
113 120
114 120
115 120
116 120
117 120
118 120
119 120
120 120
121 120
122 120
123 120
124 120
125 120
126 120
127 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533 120
65533
65533 65533
65533 102 111 111 65533
===*/

function miscDecodeTest() {
    var i;

    function test(bytes) {
        var buf = makeBuffer(bytes);
        print(Array.prototype.map.call(buf.toString(), function (v) { return v.charCodeAt(0); }).join(' '));
    }

    // All initial bytes.
    for (i = 0; i < 256; i++) {
        test([ i, 'x' ]);
    }

    // Truncated codepoint(s).
    //
    // Duktape (and Firefox) TextDecoder() -- which is used for Node.js Buffer
    // .toString() now -- emit a single replacement character for the truncated
    // sequence EC AB.  Chrome and Node.js v6.7.0 emit two.  Duktape behavior
    // matches WHATWG Encoding API.

    test([ 0xec, 0xab ]);
    test([ 0xec, 0xab, 0xec, 0xab ]);
    test([ 0xec, 0xab, 'f', 'o', 'o', 0xec, 0xab ]);
}

try {
    miscDecodeTest();
} catch (e) {
    print(e.stack || e);
}

/*===
replacement character policy
65533 65533
97 65533 65533 65533 98
65533 65533 65533
===*/

// There are a few different replacement character strategies.  Unicode
// Technical Committee recommends (http://www.unicode.org/review/pr-121.html):
//
//     Replace each maximal subpart of the ill-formed subsequence by a
//     single U+FFFD.
//
// For example, UTF-8 for U+CAFE is EC AB BE.  A byte sequence containing
// two truncated sequences EC AB EC AB could be decoded as:
//
//     1. U+FFFD (replace entire sequence)
//     2. U+FFFD U+FFFD (replace each "maximal subpart")
//     3. U+FFFD U+FFFD U+FFFD U+FFFD (replace each byte)
//
// Unicode Technical Committee recommends approach 2; Firefox does so too.
// V8 seems to use approach 3, which also affects the Node.js Buffer binding.
//
// Duktape's Buffer .toString() will thus use fewer replacement characters
// than Node.js (at least up to v6.9.1).
//
// WHATWG Encoding specification has a required algorithm for decoding (as far
// as outcomes are concerned) which provides approach 2.  So that behavior is
// required for TextDecoder() in any case.

function replacementCharacterPolicyTest() {
    // Truncated U+CAFE test.
    var buf = new Buffer(4);
    buf[0] = 0xec;
    buf[1] = 0xab;
    buf[2] = 0xec;
    buf[3] = 0xab;
    var res = buf.toString();
    print(Array.prototype.map.call(res, function (v) { return v.charCodeAt(0); }).join(' '));

    // Test from http://www.unicode.org/review/pr-121.html.
    var buf = new Buffer(8);
    buf[0] = 0x61;
    buf[1] = 0xF1;
    buf[2] = 0x80;
    buf[3] = 0x80;
    buf[4] = 0xE1;
    buf[5] = 0x80;
    buf[6] = 0xC2;
    buf[7] = 0x62;
    var res = buf.toString();
    print(Array.prototype.map.call(res, function (v) { return v.charCodeAt(0); }).join(' '));

    // Interesting special case: ED A0 80 is a CESU-8 encoded surrogate pair.
    // Because ED is not a valid initial UTF-8 byte at all, the sequence
    // generates three replacement characters.
    var buf = new Buffer(3);
    buf[0] = 0xed;
    buf[1] = 0xa0;
    buf[2] = 0x80;
    var res = buf.toString();
    print(Array.prototype.map.call(res, function (v) { return v.charCodeAt(0); }).join(' '));
}

try {
    print('replacement character policy');
    replacementCharacterPolicyTest();
} catch (e) {
    print(e.stack || e);
}

/*===
bom handling
"<U+FEFF>"
"<U+FEFF>AB"
"@<U+FEFF>AB"
===*/

function bomTest() {
    var res;

    // Just a BOM and nothing else -> Node.js doesn't strip the BOM:
    // > new Buffer(new Uint8Array([ 0xef, 0xbb, 0xbf ])).toString()
    // ''
    // > new Buffer(new Uint8Array([ 0xef, 0xbb, 0xbf ])).toString().length
    // 1
    // > new Buffer(new Uint8Array([ 0xef, 0xbb, 0xbf ])).toString().charCodeAt(0)
    // 65279

    res = new Buffer(new Uint8Array([ 0xef, 0xbb, 0xbf ])).toString();
    safePrintString(res);

    // BOM + followup characters, BOM is kept.
    res = new Buffer(new Uint8Array([ 0xef, 0xbb, 0xbf, 0x41, 0x42 ])).toString();
    safePrintString(res);

    // BOM inside a string, BOM is kept.
    res = new Buffer(new Uint8Array([ 0x40, 0xef, 0xbb, 0xbf, 0x41, 0x42 ])).toString();
    safePrintString(res);
}

try {
    print('bom handling');
    bomTest();
} catch (e) {
    print(e.stack || e);
}
