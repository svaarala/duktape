/*
 *  Specific test for Uint8ClampedArray coercion.
 */

/*@include util-typedarray.js@*/

/*===
Uint8ClampedArray coercion test
0 0 0
1 0.1 0
2 0.5 0
3 0.9 1
4 1 1
5 1.1 1
6 1.5 2
7 1.9 2
8 2 2
9 2.1 2
10 2.5 2
11 2.9 3
12 3 3
13 3.1 3
14 3.5 4
15 3.9 4
16 4 4
17 4.1 4
18 4.5 4
19 4.9 5
20 -Infinity 0
21 -100000000001000000000 0
22 -1 0
23 -0.9 0
24 -0.5 0
25 -0.1 0
26 0 0
27 255 255
28 255.1 255
29 255.5 255
30 255.9 255
31 256 255
32 1000000000 255
33 100000000000000000000 255
34 Infinity 255
35 NaN 0
===*/

function uint8ClampedArrayCoercionTest() {
    var vals = [
        // Rounding
        0, 0.1, 0.5, 0.9,
        1, 1.1, 1.5, 1.9,
        2, 2.1, 2.5, 2.9,
        3, 3.1, 3.5, 3.9,
        4, 4.1, 4.5, 4.9,

        // Clamping
        Number.NEGATIVE_INFINITY,
        -1e20
        -1e9,  // within 32-bit
        -1,
        -0.9,
        -0.5,
        -0.1,
        -0,
        255,
        255.1,
        255.5,
        255.9,
        256,
        1e9,
        1e20,
        Number.POSITIVE_INFINITY,
        Number.NaN
    ];
    var b = new ArrayBuffer(vals.length);
    var v1 = new Uint8ClampedArray(b);
    var i;

    // http://www.ecma-international.org/ecma-262/6.0/index.html#sec-touint8clamp
    // https://www.khronos.org/registry/typedarray/specs/latest/#7.1
    // https://www.khronos.org/registry/typedarray/specs/latest/#3

    for (i = 0; i < vals.length; i++) {
        v1[i] = vals[i];
    }

    for (i = 0; i < v1.byteLength; i++) {
        print(i, vals[i], v1[i]);
    }
}

try {
    print('Uint8ClampedArray coercion test');
    uint8ClampedArrayCoercionTest();
} catch (e) {
    print(e.stack || e);
}
