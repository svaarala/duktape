/*
 *  TypedArray.prototype.set()
 *
 *  This is a very tricky method to implement:
 *
 *    - Source may be a generic array or a TypedArray.
 *
 *    - Source may be a view pointing to the same underlying buffer.
 *
 *    - The operation is not a byte copy but conceptually values are
 *      read from the source array and written to the target array,
 *      performing necessary coercions.  For example, the source may
 *      be Int8 and target Uint32.
 *
 *    - The set() operation may overlap in the same underlying buffer.
 *      Because the target element size may be larger, the byte ranges
 *      may differ in size (e.g. Int8 source of 4 bytes total maps to
 *      Uint32 target of 16 bytes total) so that the target overlaps
 *      the source regardless of which order is used for copying the
 *      values; a temporary buffer is necessarily needed.
 *
 *    - Internal implementation issue: even if duk_hbufferobject 'buf'
 *      references are different, there's no guarantee that there's no
 *      overlap.  Although fixed and dynamic buffers never overlap,
 *      external buffers may very well overlap (intentionally).  So
 *      the only reliable way to detect overlap is pointer comparison.
 *
 *  On the other hand it's important that the common case is handled as a
 *  byte copy.  Note that elements of the same byte size are still not
 *  always compatible.  For example:
 *
 *    - Writing from Uint8Clamped source to Int8 is compatible because
 *      there's no byte level change in writing an int8.
 *
 *    - But, writing from Int8 source to Uint8Clamped target is *not*
 *      compatible, e.g. -1 source value would map to 0x00 (not 0xFF).
 *
 *    - Similarly integer/float sources cannot be trivially mixed.
 *
 *  The implementation can be simplistic and use byte copy only when the
 *  element type is identical (assuming coercion is then idempotent) or
 *  explicitly check for compatible pairs.
 *
 *  This testcase must cover all of these combinations.
 */

/*@include util-typedarray.js@*/
/*@include util-checksum-string.js@*/

/*---
{
    "custom": true,
    "endianness": "little"
}
---*/

function getFilledBuffer(size) {
    var buf = new ArrayBuffer(size);
    for (i = 0; i < buf.byteLength; i++) {
        (new Uint8Array(buf))[i] = 0x40 + i;
    }
    return buf;
}

/*===
byte copy test
0 0 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 16 4041424344454647404142434445464748494a4b4c4d4e4f58595a5b5c5d5e5f
0 17 RangeError
0 8 40414243444546474041424344454647505152535455565758595a5b5c5d5e5f
0 12 4041424344454647404142434445464748494a4b5455565758595a5b5c5d5e5f
8 12 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 12 40414243444546474c4d4e4f50515253545556575455565758595a5b5c5d5e5f
16 12 4041424344454647505152535455565758595a5b5455565758595a5b5c5d5e5f
24 8 404142434445464758595a5b5c5d5e5f505152535455565758595a5b5c5d5e5f
===*/

/* When the views are copy compatible (they have the same element size,
 * there is no conflicting coercion, internal buffers are valid, etc),
 * Duktape uses a memmove() to handle .set().  Exercise that code path.
 */

function byteCopyTest() {
    var buf;
    var v1, v2;

    [
        [0, 0],
        [0, 16],
        [0, 17],  // doesn't fit, RangeError

        [0, 8],   // doesn't overlap
        [0, 12],  // overlaps target from beginning (bytes [8,12[)
        [8, 12],  // overlaps target 1:1 but not to end of buffer
        [12, 12], // overlaps target from end (bytes [12,24[)
        [16, 12], // overlaps target from end (bytes [16,24[)
        [24, 8],  // doesn't overlap
    ].forEach(function (range) {
        try {
            buf = getFilledBuffer(32);         // [0,32[
            v1 = new Uint8Array(buf, 8, 16);   // [8,24[
            v2 = new Uint8Array(buf, range[0], range[1]);
            //print('v1', v1.length, v1.byteLength, v1.byteOffset);
            //print('v2', v2.length, v2.byteLength, v2.byteOffset);
            v1.set(v2);
            print(range[0], range[1], printableTypedArray(buf));
        } catch (e) {
            print(range[0], range[1], e.name);
        }
    });
}

try {
    print('byte copy test');
    byteCopyTest();
} catch (e) {
    print(e.stack || e);
}

/*===
fast copy test
0 0 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 4 40414243444546474000000041000000420000004300000058595a5b5c5d5e5f
0 5 RangeError
0 2 40414243444546474000000041000000505152535455565758595a5b5c5d5e5f
0 3 40414243444546474000000041000000420000005455565758595a5b5c5d5e5f
8 3 404142434445464748000000490000004a0000005455565758595a5b5c5d5e5f
12 3 40414243444546474c0000004d0000004e0000005455565758595a5b5c5d5e5f
16 3 40414243444546475000000051000000520000005455565758595a5b5c5d5e5f
24 2 40414243444546475800000059000000505152535455565758595a5b5c5d5e5f
===*/

/* When both source and target arguments are TypedArrays, Duktape uses a
 * semi-fast copy loop where coercions are needed but we access the elements
 * from memory directly.
 *
 * Because element sizes are not necessarily equal, semi-fast copy needs
 * to handle an overlap case where a temporary copy is needed because the
 * target overlaps the source "from both ends".
 */

function fastCopyTest() {
    var buf;
    var v1, v2;

    [
        [0, 0],
        [0, 4],
        [0, 5],   // doesn't fit, RangeError

        [0, 2],   // doesn't overlap
        [0, 3],   // overlaps target from beginning (bytes [8,12[)
        [8, 3],   // overlaps target 1:1 but not to end of buffer
        [12, 3],  // overlaps target from end (bytes [12,24[)
        [16, 3],  // overlaps target from end (bytes [16,24[)
        [24, 2],  // doesn't overlap
    ].forEach(function (range) {
        try {
            buf = getFilledBuffer(32);         // [0,32[
            v1 = new Int32Array(buf, 8, 4);    // [8,24[
            v2 = new Int8Array(buf, range[0], range[1]);
            //print('v1', v1.length, v1.byteLength, v1.byteOffset);
            //print('v2', v2.length, v2.byteLength, v2.byteOffset);
            v1.set(v2);
            print(range[0], range[1], printableTypedArray(buf));
        } catch (e) {
            print(range[0], range[1], e.name);
        }
    });
}

try {
    print('fast copy test');
    fastCopyTest();
} catch (e) {
    print(e.stack || e);
}

/*===
slow copy test
 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
-1,2,1193046,3735928559 4041424344454647ffff02005634efbe505152535455565758595a5b5c5d5e5f
1,2,3,4,5,6,7,8 40414243444546470100020003000400050006000700080058595a5b5c5d5e5f
1,2,3,4,5,6,7,8,9 RangeError
[object Object] 4041424344454647ffff4100ff014e4f505152535455565758595a5b5c5d5e5f
xy1234 40414243444546470000000001000200030004005455565758595a5b5c5d5e5f
===*/

/* A slow copy is used for Array-like values. */

function slowCopyTest() {
    var buf;
    var v1, v2;

    [
        [],
        [ -1, 2, 0x123456, 0xdeadbeef ],
        [ '1', '2', 3, 4, '5', '6', 7, '8' ],

        // Out of bounds (target is 16 bytes, 8 Int16).
        [ '1', '2', 3, 4, '5', '6', 7, '8', 9 ],

        { '0': -1, '1': 0x41, '2': 0x1ff, '3': 0x55, length: 3 },

        // A String object is Array-like but not in the intuitive way:
        // the indexed values are strings which get number coerced;
        // most characters will coerce to 0, but e.g. '1', '2', etc
        // will coerce to numbers.
        new String('xy1234'),

    ].forEach(function (arraylike) {
        try {
            buf = getFilledBuffer(32);         // [0,32[
            v1 = new Int16Array(buf, 8, 8);    // [8,24[
            v1.set(arraylike);
            print(arraylike, printableTypedArray(buf));
        } catch (e) {
            print(arraylike, e.name);
        }
    });
}

try {
    print('slow copy test');
    slowCopyTest();
} catch (e) {
    print(e.stack || e);
}

/*===
int8 to uint8clamped test
|0000000000| |01d802807f|
|010002007f| |01d802807f|
|000001d802807f|
|010002007f807f|
|01d802807f0000|
|01d8010002007f|
===*/

/* Specific test for Int8 -> Uint8Clamped set(), which cannot use the byte
 * copy path because it needs coercion (e.g. -1 to 0x00).
 */

function int8ToUint8ClampedTest() {
    var b1, b2;
    var v1, v2;

    // Simple case.

    b1 = new ArrayBuffer(5);
    b2 = new ArrayBuffer(5);
    v1 = new Uint8ClampedArray(b1);
    v2 = new Int8Array(b2);

    v2.set([1, -40, 2, -128, 127]);
    print(Duktape.enc('jx', b1), Duktape.enc('jx', b2));
    v1.set(v2);
    print(Duktape.enc('jx', b1), Duktape.enc('jx', b2));

    // Overlap case 1

    b1 = new ArrayBuffer(7);
    v1 = new Uint8ClampedArray(b1, 0, 5);
    v2 = new Int8Array(b1, 2, 5);

    v2.set([1, -40, 2, -128, 127]);
    print(Duktape.enc('jx', b1));
    v1.set(v2);
    print(Duktape.enc('jx', b1));

    // Overlap case 2

    b1 = new ArrayBuffer(7);
    v1 = new Uint8ClampedArray(b1, 2, 5);
    v2 = new Int8Array(b1, 0, 5);

    v2.set([1, -40, 2, -128, 127]);
    print(Duktape.enc('jx', b1));
    v1.set(v2);
    print(Duktape.enc('jx', b1));
}

try {
    print('int8 to uint8clamped test');
    int8ToUint8ClampedTest();
} catch (e) {
    print(e.stack || e);
}

/*===
offset test
0 0 0 0 5f728598abbed1e4f70a1d304356697c102132435465768798a9bacbdcedfe0f 5f728598abbed1e4f70a1d304356697c
0 0 1 1 001122335f728598abbed1e4f70a1d304356697c5465768798a9bacbdcedfe0f 5f728598abbed1e4f70a1d304356697c
0 0 2 2 00112233445566775f728598abbed1e4f70a1d304356697c98a9bacbdcedfe0f 5f728598abbed1e4f70a1d304356697c
0 0 3 3 00112233445566778899aabb5f728598abbed1e4f70a1d304356697cdcedfe0f 5f728598abbed1e4f70a1d304356697c
0 0 4 4 00112233445566778899aabbccddeeff5f728598abbed1e4f70a1d304356697c 5f728598abbed1e4f70a1d304356697c
0 1 0 0 5f7200008598ffffabbeffffd1e4ffff102132435465768798a9bacbdcedfe0f 5f728598abbed1e4f70a1d304356697c
0 1 1 1 001122335f7200008598ffffabbeffffd1e4ffff5465768798a9bacbdcedfe0f 5f728598abbed1e4f70a1d304356697c
0 1 2 2 00112233445566775f7200008598ffffabbeffffd1e4ffff98a9bacbdcedfe0f 5f728598abbed1e4f70a1d304356697c
0 1 3 3 00112233445566778899aabb5f7200008598ffffabbeffffd1e4ffffdcedfe0f 5f728598abbed1e4f70a1d304356697c
0 1 4 4 00112233445566778899aabbccddeeff5f7200008598ffffabbeffffd1e4ffff 5f728598abbed1e4f70a1d304356697c
0 2 0 0 5f000000720000008500000098000000102132435465768798a9bacbdcedfe0f 5f728598abbed1e4f70a1d304356697c
0 2 1 1 001122335f0000007200000085000000980000005465768798a9bacbdcedfe0f 5f728598abbed1e4f70a1d304356697c
0 2 2 2 00112233445566775f00000072000000850000009800000098a9bacbdcedfe0f 5f728598abbed1e4f70a1d304356697c
0 2 3 3 00112233445566778899aabb5f000000720000008500000098000000dcedfe0f 5f728598abbed1e4f70a1d304356697c
0 2 4 4 00112233445566778899aabbccddeeff5f000000720000008500000098000000 5f728598abbed1e4f70a1d304356697c
===*/

/* Offset argument is in -elements-, not bytes. */

function offsetTest() {
    var b1 = new ArrayBuffer(32);
    var v1 = new Uint32Array(b1);

    var b2 = new ArrayBuffer(16);
    var v2a = new Uint32Array(b2, 0, 4);
    var v2b = new Int16Array(b2, 0, 4);
    var v2c = new Uint8Array(b2, 0, 4);

    function initBuffers() {
        var i;
        var v;

        // Ensure values include both positive/negative values
        // for integer views.
        v = new Uint8Array(b1);
        for (i = 0; i < v.length; i++) {
            v[i] = i * 0x11;
        }
        v = new Uint8Array(b2);
        for (i = 0; i < v.length; i++) {
            v[i] = (i + 5) * 0x13;
        }
    }

    [ v1 ].forEach(function (thisValue, idx1) {
        [ v2a, v2b, v2c ].forEach(function (argValue, idx2) {
            [ 0, 1, 2, 3, 4 ].forEach(function (offset, idx3) {
                try {
                    initBuffers();
                    thisValue.set(argValue, offset);
                    print(idx1, idx2, idx3, offset, printableTypedArray(b1), printableTypedArray(b2));
                } catch (e) {
                    print(idx1, idx2, idx3, offset, e.name, printableTypedArray(b1), printableTypedArray(b2));
                }
            });
        });
    });
}

try {
    print('offset test');
    offsetTest();
} catch (e) {
    print(e.stack || e);
}

/*===
TypedArray set() bruteforce test
0 22338704
1 22461178
2 22459332
3 22421068
4 21623155
5 22078325
6 21761282
7 22338704
8 22461178
9 22459332
10 22421068
11 21623155
12 22078325
13 21761282
14 22584786
15 22695412
16 22684820
17 22634362
18 21889721
19 22298137
20 22025681
21 22593358
22 22644609
23 22626885
24 21476770
25 22470391
26 22296551
27 22593358
28 22644609
29 22626885
30 21476770
31 22470391
32 22296551
33 22731620
34 22759992
35 22138961
36 22626746
37 22570924
38 22731620
39 22759992
40 22138961
41 22626746
42 22570924
43 22786301
44 22778166
45 22336173
46 22679603
47 22662385
48 22778756
49 22521149
50 22667701
51 22739018
===*/

function typedArraySetBruteForceTest() {
    /*
     *  For the brute force test, use a few buffers, and views that
     *  will sometimes share a buffer and sometimes not.
     *
     *  The test output is megabytes long so we print only a digest.
     *  If there's a test failure, enable the printouts manually.
     */

    var b1 = new ArrayBuffer(11);
    var b2 = new ArrayBuffer(24);

    function initBuffers() {
        var i;
        var v;

        // Ensure values include both positive/negative values
        // for integer views.
        v = new Uint8Array(b1);
        for (i = 0; i < v.length; i++) {
            v[i] = i * 0x11;
        }
        v = new Uint8Array(b2);
        for (i = 0; i < v.length; i++) {
            v[i] = (i + 5) * 0x13;
        }
    }

    var views = [
        // 0
        new Int8Array(b1),
        new Int8Array(b1, 1, 8),
        new Int8Array(b1, 2, 8),
        new Int8Array(b1, 3, 8),
        new Int8Array(b2),
        new Int8Array(b2, 2, 8),
        new Int8Array(b2, 4, 12),

        // 7
        new Uint8Array(b1),
        new Uint8Array(b1, 1, 8),
        new Uint8Array(b1, 2, 8),
        new Uint8Array(b1, 3, 8),
        new Uint8Array(b2),
        new Uint8Array(b2, 2, 8),
        new Uint8Array(b2, 4, 12),

        // 14
        new Uint8ClampedArray(b1),
        new Uint8ClampedArray(b1, 1, 8),
        new Uint8ClampedArray(b1, 2, 8),
        new Uint8ClampedArray(b1, 3, 8),
        new Uint8ClampedArray(b2),
        new Uint8ClampedArray(b2, 2, 8),
        new Uint8ClampedArray(b2, 4, 12),

        // 21
        new Int16Array(b1, 0, 5),  // can't map 11 bytes
        new Int16Array(b1, 0, 4),
        new Int16Array(b1, 2, 4),
        new Int16Array(b2),
        new Int16Array(b2, 2, 4),
        new Int16Array(b2, 4, 6),

        // 27
        new Uint16Array(b1, 0, 5),  // can't map 11 bytes
        new Uint16Array(b1, 0, 4),
        new Uint16Array(b1, 2, 4),
        new Uint16Array(b2),
        new Uint16Array(b2, 2, 4),
        new Uint16Array(b2, 4, 6),

        new Int32Array(b1, 0, 2),  // can't map 11 bytes
        new Int32Array(b1, 4, 1),
        new Int32Array(b2),
        new Int32Array(b2, 4, 2),
        new Int32Array(b2, 8, 3),

        new Uint32Array(b1, 0, 2),  // can't map 11 bytes
        new Uint32Array(b1, 4, 1),
        new Uint32Array(b2),
        new Uint32Array(b2, 4, 2),
        new Uint32Array(b2, 8, 3),

        new Float32Array(b1, 0, 2),  // can't map 11 bytes
        new Float32Array(b1, 4, 1),
        new Float32Array(b2),
        new Float32Array(b2, 4, 2),
        new Float32Array(b2, 8, 3),

        new Float64Array(b1, 0, 1),  // can't map 11 bytes
        new Float64Array(b2),
        new Float64Array(b2, 8, 2),
        new Float64Array(b2, 16, 1),
    ];

    var offsets = [
        'NONE', -1, 0, 1, 2, 4, 8
    ];

    views.forEach(function (thisValue, idx1) {
        var tmp = [];
        views.forEach(function (argValue, idx2) {
            offsets.forEach(function (offset, idx3) {
                try {
                    initBuffers();
                    if (offset === 'NONE') {
                        thisValue.set(argValue);
                    } else {
                        thisValue.set(argValue, offset);
                    }
                    tmp.push('success');
                    tmp.push(printableTypedArray(b1));
                    tmp.push(printableTypedArray(b2));

                    //print(idx1, idx2, idx3, printableTypedArray(b1), printableTypedArray(b2));
                } catch (e) {
                    tmp.push(e.name);
                    tmp.push(printableTypedArray(b1));
                    tmp.push(printableTypedArray(b2));
                    //print(idx1, idx2, idx3, e.name, printableTypedArray(b1), printableTypedArray(b2));
                }
            });
        });
        print(idx1, checksumString(tmp.join('\n')));
    });
}

try {
    print('TypedArray set() bruteforce test');
    typedArraySetBruteForceTest();
} catch (e) {
    print(e.stack || e);
}

/*===
TypedArray set() retval test
undefined undefined
===*/

function typedArraySetRetvalTest() {
    // Khronos spec says "void" return, so undefined.

    var v = new Uint8Array(16);
    var ret = v.set([ 1, 2, 3 ]);
    print(typeof ret, ret);
}

try {
    print('TypedArray set() retval test');
    typedArraySetRetvalTest();
} catch (e) {
    print(e.stack || e);
}
