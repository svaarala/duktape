/*
 *  Some base-64 tests.
 */

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

var t;

function encPrint(x) {
    print(Duktape.enc('base64', x));
}

function decPrint(x) {
    print(bufferToStringRaw(Duktape.dec('base64', x)));
}

/*===

Zg==
Zm8=
Zm9v
Zm9vYg==
Zm9vYmE=
Zm9vYmFy
===*/

encPrint('');
encPrint('f');
encPrint('fo');
encPrint('foo');
encPrint('foob');
encPrint('fooba');
encPrint('foobar');

/*===

f
fo
foo
foob
fooba
foobar
===*/

decPrint('');
decPrint('Zg==');
decPrint('Zm8=');
decPrint('Zm9v');
decPrint('Zm9vYg==');
decPrint('Zm9vYmE=');
decPrint('Zm9vYmFy');

/*===
Zm9v4Yi0
102 111 111 4660
===*/

/* A string is UTF-8 encoded and then base-64 encoded. */

// U+1234 -> 0xe1 0x88 0xb4
encPrint('foo\u1234');

t = bufferToStringRaw(Duktape.dec('base64', 'Zm9v4Yi0'));
print(t.charCodeAt(0), t.charCodeAt(1), t.charCodeAt(2), t.charCodeAt(3));

/*===
Zg==b28=
foo
Zm8=bw==
foo
===*/

/* The current decoder is lenient in that it allows concatenated base64
 * documents to be decoded (even when there is intervening padding).
 */

t = Duktape.enc('base64', 'f') + Duktape.enc('base64', 'oo');
print(t);
decPrint(t);

t = Duktape.enc('base64', 'fo') + Duktape.enc('base64', 'o');
print(t);
decPrint(t);

/*===
Zm9vYmFycXV1eA==
foobarquux
foobarquux
foobarquux
===*/

/* The current decoder allows ASCII whitespace (CR, LF, TAB, SPACE) at
 * any position.
 */

t = Duktape.enc('base64', 'foobarquux');
print(t);
decPrint(t);

decPrint('Zm9vYmFycXV1eA==\n');
decPrint('Z m\t9\rv\nY  \r\n\t m  \n\n FycX \r\r\nV1eA =\t\t=\n');

/*===
f
f
fo
fo
===*/

/* The current decoder will not check that unused bits of a partial
 * group are actually zero.  This matches e.g. Python behavior.
 */

try {
    decPrint('Zg==');  // standard
    decPrint('Zh==');  // non-zero unused bits
} catch (e) {
    print(e.name);
}

try {
    decPrint('Zm8=');  // standard
    decPrint('Zm9=');  // non-zero unused bits
} catch (e) {
    print(e.name);
}


/*===
xy
xy
===*/

/* Base64 strings without end padding are accepted since Duktape 2.3.
 * For instance 'xy' encodes into 'eHk=' which is unambiguously decodeable
 * from 'eHk'.
 */

try {
    decPrint('eHk=');
} catch(e) {
    print(e.name);
}

try {
    decPrint('eHk');
} catch(e) {
    print(e.name);
}

/*===
Zg==
b28=

foo
===*/

/* The current decoder also allows ASCII whitespace characters */

t = Duktape.enc('base64', 'f') + '\n' + Duktape.enc('base64', 'oo') + '\n';
print(t);
decPrint(t);

/*===
TypeError
===*/

/* Non-base64 characters will not be accepted */

try {
    decPrint('b28?');
} catch (e) {
    print(e.name);
}

/*===
0 
1 wA==
2 wME=
3 wMHC
4 wMHCww==
5 wMHCw8Q=
6 wMHCw8TF
7 wMHCw8TFxg==
8 wMHCw8TFxsc=
9 wMHCw8TFxsfI
10 wMHCw8TFxsfIyQ==
11 wMHCw8TFxsfIyco=
12 wMHCw8TFxsfIycrL
13 wMHCw8TFxsfIycrLzA==
14 wMHCw8TFxsfIycrLzM0=
15 wMHCw8TFxsfIycrLzM3O
16 wMHCw8TFxsfIycrLzM3Ozw==
17 wMHCw8TFxsfIycrLzM3Oz9A=
18 wMHCw8TFxsfIycrLzM3Oz9DR
19 wMHCw8TFxsfIycrLzM3Oz9DR0g==
20 wMHCw8TFxsfIycrLzM3Oz9DR0tM=
21 wMHCw8TFxsfIycrLzM3Oz9DR0tPU
22 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1Q==
23 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dY=
24 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX
25 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2A==
26 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nk=
27 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna
28 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna2w==
29 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29w=
30 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd
31 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3g==
32 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t8=
33 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g
34 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4Q==
35 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eI=
36 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj
37 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5A==
38 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OU=
39 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm
40 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5w==
41 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+g=
42 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp
43 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6g==
44 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6us=
45 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs
46 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7Q==
47 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e4=
48 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v
49 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8A==
50 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PE=
51 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy
52 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8w==
53 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/Q=
54 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T1
55 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19g==
56 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vc=
57 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4
58 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+Q==
59 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fo=
60 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7
61 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/A==
62 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P0=
63 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+
64 wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==
===*/

/* Input lengths up to 64.  In current fast path input lengths below a certain
 * limit (16 now) skip the fast path so exercise both cases.  Fast path unroll
 * is for 12-byte input chunks.
 */

function testInputLengths() {
    var len, i, u8;

    for (len = 0; len <= 64; len++) {
        u8 = new Uint8Array(len);
        for (i = 0; i < len; i++) {
            u8[i] = 0xc0 + i;
        }
        print(len, Duktape.enc('base64', u8));
    }
}
try {
    testInputLengths();
} catch (e) {
    print(e.name);
}
