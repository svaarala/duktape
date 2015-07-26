/*
 *  Testcase matching Khronos specification Section 10 example to some extent.
 */

/*@include util-typedarray.js@*/

/*===
section 10 example test
0 0 1 2 3
4 4 5 6 7
8 0 1 2 3
12 4 5 6 7
16 0 1 2 3
20 4 5 6 7
24 0 1 2 3
28 4 5 6 7
32 0 1 2 3
36 4 5 6 7
40 0 1 2 3
44 4 5 6 7
48 0 1 2 3
52 4 5 6 7
56 0 1 2 3
60 4 5 6 7
64 0 1 2 3
68 4 5 6 7
72 0 1 2 3
76 4 5 6 7
80 0 1 2 3
84 4 5 6 7
88 0 1 2 3
92 4 5 6 7
96 0 1 2 3
100 4 5 6 7
104 0 1 2 3
108 4 5 6 7
112 0 1 2 3
116 4 5 6 7
120 0 1 2 3
124 4 5 6 7
16 64 16 52
4 16
true
true
===*/

function section10ExampleTest() {
    var i, j;
    var tmp;
    var tmpView;

    // "Filling each 8 consecutive floats of the new array"
    //
    // Note: there's a bug in the original code: the subarray starting
    // position marches on by 1 (not by 8) which is probably not intended.
    // Fixed below.

    var f32s = new Float32Array(128);
    for (i = 0; i < 128; i += 8) {
        var sub_f32s = f32s.subarray(i, i + 8);
        for (j = 0; j < 8; ++j) {
            sub_f32s[j] = j;
        }
    }
    for (i = 0; i < 128; i += 4) {
        print(i, f32s[i], f32s[i + 1], f32s[i + 2], f32s[i + 3]);
    }

    // "Interleaved array types test"

    var elementSize = 3 * Float32Array.BYTES_PER_ELEMENT + 4 * Uint8Array.BYTES_PER_ELEMENT;
    var buffer = new ArrayBuffer(4 * elementSize);
    var coords = new Float32Array(buffer, 0);
    var colors = new Uint8Array(buffer, 3 * Float32Array.BYTES_PER_ELEMENT);

    print(elementSize, buffer.byteLength, coords.length, colors.length);

    var coordOffset = elementSize / Float32Array.BYTES_PER_ELEMENT;
    var colorOffset = elementSize / Uint8Array.BYTES_PER_ELEMENT;

    print(coordOffset, colorOffset);

    coords[0] = coords[1] = coords[2] = 1.0; // The first point's three coordinate values
    colors[0] = colors[1] = colors[2] = colors[3] = 255; // The first point's four colors

    var N = 2;
    coords[0+N*coordOffset] = 5.0; // The Nth point's first coordinate value
    colors[0+N*colorOffset] = 128; // The Nth point's first color value

    i = 1;
    j = 2;
    coords[i+N*coordOffset] = 6.0; // The Nth point's i coordinate value;
    colors[j+N*colorOffset] = 200; // The Nth point's j color value

    // Expect string depends on Float32 endianness

    var expect_little = [
        0x00, 0x00, 0x80, 0x3f,  // 1.0
        0x00, 0x00, 0x80, 0x3f,  // 1.0
        0x00, 0x00, 0x80, 0x3f,  // 1.0
        0xff, 0xff, 0xff, 0xff,  // 255, 255, 255, 255
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x00, 0x00, 0x00, 0x00,  // 0, 0, 0, 0
        0x00, 0x00, 0xa0, 0x40,  // 5.0
        0x00, 0x00, 0xc0, 0x40,  // 6.0
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x80, 0x00, 0xc8, 0x00,  // 128, 0, 200, 0
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x00, 0x00, 0x00, 0x00  // 0, 0, 0, 0
    ].join(' ');
    var expect_big = [
        0x3f, 0x80, 0x00, 0x00,  // 1.0
        0x3f, 0x80, 0x00, 0x00,  // 1.0
        0x3f, 0x80, 0x00, 0x00,  // 1.0
        0xff, 0xff, 0xff, 0xff,  // 255, 255, 255, 255
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x00, 0x00, 0x00, 0x00,  // 0, 0, 0, 0
        0x40, 0xa0, 0x00, 0x00,  // 5.0
        0x40, 0xc0, 0x00, 0x00,  // 6.0
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x80, 0x00, 0xc8, 0x00,  // 128, 0, 200, 0
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x00, 0x00, 0x00, 0x00,  // 0.0
        0x00, 0x00, 0x00, 0x00  // 0, 0, 0, 0
    ].join(' ');

    tmp = [];
    tmpView = new Uint8Array(buffer);
    for (i = 0; i < tmpView.length; i++) {
        tmp.push(tmpView[i]);
    }
    tmp = tmp.join(' ');
    //print(tmp);
    if (integerEndianness == 'big') {
        print(tmp === expect_big);
    } else if (integerEndianness == 'little') {
        print(tmp === expect_little);
    } else {
        throw new Error('internal error');
    }

    // "Slicing a large array into multiple regions"

    var buffer = new ArrayBuffer(1024);
    var floats = new Float32Array(buffer, 0, 128);    // 128x4 = 512 bytes
    var shorts = new Uint16Array(buffer, 512, 128);   // 128x2 = 256 bytes
    var bytes = new Uint8Array(buffer, shorts.byteOffset + shorts.byteLength);  // 256 bytes

    for (i = 0; i < floats.length; i++) {
        floats[i] = 5.0;
    }
    for (i = 0; i < shorts.length; i++) {
        shorts[i] = 0xbeef;
    }
    for (i = 0; i < bytes.length; i++) {
        bytes[i] = 0xa5;
    }

    // Expect string depends on endianness
    var expect = [];
    if (integerEndianness == 'big') {
        for (i = 0; i < floats.length; i++) {
            expect.push(0x40, 0xa0, 0x00, 0x00);
        }
        for (i = 0; i < shorts.length; i++) {
            expect.push(0xbe, 0xef);
        }
        for (i = 0; i < bytes.length; i++) {
            expect.push(0xa5);
        }
    } else if (integerEndianness == 'little') {
        for (i = 0; i < floats.length; i++) {
            expect.push(0x00, 0x00, 0xa0, 0x40);
        }
        for (i = 0; i < shorts.length; i++) {
            expect.push(0xef, 0xbe);
        }
        for (i = 0; i < bytes.length; i++) {
            expect.push(0xa5);
        }
    } else {
        throw new Error('internal error');
    }
    expect = expect.join(' ');

    tmp = [];
    tmpView = new Uint8Array(buffer);
    for (i = 0; i < tmpView.length; i++) {
        tmp.push(tmpView[i]);
    }
    tmp = tmp.join(' ');
    //print(tmp);
    //print(expect);
    print(tmp === expect);
}

try {
    print('section 10 example test');
    section10ExampleTest();
} catch (e) {
    print(e.stack || e);
}
