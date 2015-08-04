/*
 *  TypedArray subarray() method
 */

/*@include util-typedarray.js@*/

/*===
basic test
8 bytes: 1011121314151617
8 bytes: 1011121314151617
object true
2 bytes: 1314
object true
8 bytes: 101112dead151617
8 bytes: 101112dead151617
2 bytes: dead
===*/

function subarrayBasicTest() {
    var i;

    var b = new ArrayBuffer(8);
    var v0 = new Uint8Array(b);
    for (i = 0; i < 8; i++) {
        v0[i] = 0x10 + i;
    }
    printTypedArray(b);

    var v1 = new Uint8Array(b);
    printTypedArray(v1);
    print(typeof v1.buffer, v1.buffer === b);

    var v2 = v1.subarray(3, 5);
    printTypedArray(v2);
    print(typeof v2.buffer, v2.buffer === b);

    v2[0] = 0xde;  // ok, map to b[3]
    v2[1] = 0xad;  // ok, map to b[4]
    v2[2] = 0xbe;  // out of subarray, becomes a concrete property on v2
    v2[3] = 0xef;  // out of subarray, becomes a concrete property on v2

    printTypedArray(b);
    printTypedArray(v1);
    printTypedArray(v2);
}

try {
    print('basic test');
    subarrayBasicTest();
} catch (e) {
    print(e.stack || e);
}

/*===
bruteforce test
0 0 0 [object Int8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 0 1 [object Int8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 0 2 [object Int8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 0 3 [object Int8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 0 4 [object Int8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 0 5 [object Int8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 0 6 [object Int8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 0 7 [object Int8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 1 0 [object Int8Array] 5d5e5f
0 1 1 [object Int8Array] 
0 1 2 [object Int8Array] 5d5e
0 1 3 [object Int8Array] 
0 1 4 [object Int8Array] 
0 1 5 [object Int8Array] 
0 1 6 [object Int8Array] 
0 1 7 [object Int8Array] 
0 2 0 [object Int8Array] 5f
0 2 1 [object Int8Array] 
0 2 2 [object Int8Array] 
0 2 3 [object Int8Array] 
0 2 4 [object Int8Array] 
0 2 5 [object Int8Array] 
0 2 6 [object Int8Array] 
0 2 7 [object Int8Array] 
0 3 0 [object Int8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 3 1 [object Int8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c
0 3 2 [object Int8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e
0 3 3 [object Int8Array] 
0 3 4 [object Int8Array] 40
0 3 5 [object Int8Array] 4041
0 3 6 [object Int8Array] 404142
0 3 7 [object Int8Array] 40414243444546474849
0 4 0 [object Int8Array] 4142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 4 1 [object Int8Array] 4142434445464748494a4b4c4d4e4f505152535455565758595a5b5c
0 4 2 [object Int8Array] 4142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e
0 4 3 [object Int8Array] 
0 4 4 [object Int8Array] 
0 4 5 [object Int8Array] 41
0 4 6 [object Int8Array] 4142
0 4 7 [object Int8Array] 414243444546474849
0 5 0 [object Int8Array] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 5 1 [object Int8Array] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c
0 5 2 [object Int8Array] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e
0 5 3 [object Int8Array] 
0 5 4 [object Int8Array] 
0 5 5 [object Int8Array] 
0 5 6 [object Int8Array] 42
0 5 7 [object Int8Array] 4243444546474849
0 6 0 [object Int8Array] 434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 6 1 [object Int8Array] 434445464748494a4b4c4d4e4f505152535455565758595a5b5c
0 6 2 [object Int8Array] 434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e
0 6 3 [object Int8Array] 
0 6 4 [object Int8Array] 
0 6 5 [object Int8Array] 
0 6 6 [object Int8Array] 
0 6 7 [object Int8Array] 43444546474849
0 7 0 [object Int8Array] 4a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
0 7 1 [object Int8Array] 4a4b4c4d4e4f505152535455565758595a5b5c
0 7 2 [object Int8Array] 4a4b4c4d4e4f505152535455565758595a5b5c5d5e
0 7 3 [object Int8Array] 
0 7 4 [object Int8Array] 
0 7 5 [object Int8Array] 
0 7 6 [object Int8Array] 
0 7 7 [object Int8Array] 
1 0 0 [object Int8Array] 45464748494a4b4c4d4e4f5051
1 0 1 [object Int8Array] 45464748494a4b4c4d4e4f5051
1 0 2 [object Int8Array] 45464748494a4b4c4d4e4f5051
1 0 3 [object Int8Array] 45464748494a4b4c4d4e4f5051
1 0 4 [object Int8Array] 45464748494a4b4c4d4e4f5051
1 0 5 [object Int8Array] 45464748494a4b4c4d4e4f5051
1 0 6 [object Int8Array] 45464748494a4b4c4d4e4f5051
1 0 7 [object Int8Array] 45464748494a4b4c4d4e4f5051
1 1 0 [object Int8Array] 4f5051
1 1 1 [object Int8Array] 
1 1 2 [object Int8Array] 4f50
1 1 3 [object Int8Array] 
1 1 4 [object Int8Array] 
1 1 5 [object Int8Array] 
1 1 6 [object Int8Array] 
1 1 7 [object Int8Array] 
1 2 0 [object Int8Array] 51
1 2 1 [object Int8Array] 
1 2 2 [object Int8Array] 
1 2 3 [object Int8Array] 
1 2 4 [object Int8Array] 
1 2 5 [object Int8Array] 
1 2 6 [object Int8Array] 
1 2 7 [object Int8Array] 
1 3 0 [object Int8Array] 45464748494a4b4c4d4e4f5051
1 3 1 [object Int8Array] 45464748494a4b4c4d4e
1 3 2 [object Int8Array] 45464748494a4b4c4d4e4f50
1 3 3 [object Int8Array] 
1 3 4 [object Int8Array] 45
1 3 5 [object Int8Array] 4546
1 3 6 [object Int8Array] 454647
1 3 7 [object Int8Array] 45464748494a4b4c4d4e
1 4 0 [object Int8Array] 464748494a4b4c4d4e4f5051
1 4 1 [object Int8Array] 464748494a4b4c4d4e
1 4 2 [object Int8Array] 464748494a4b4c4d4e4f50
1 4 3 [object Int8Array] 
1 4 4 [object Int8Array] 
1 4 5 [object Int8Array] 46
1 4 6 [object Int8Array] 4647
1 4 7 [object Int8Array] 464748494a4b4c4d4e
1 5 0 [object Int8Array] 4748494a4b4c4d4e4f5051
1 5 1 [object Int8Array] 4748494a4b4c4d4e
1 5 2 [object Int8Array] 4748494a4b4c4d4e4f50
1 5 3 [object Int8Array] 
1 5 4 [object Int8Array] 
1 5 5 [object Int8Array] 
1 5 6 [object Int8Array] 47
1 5 7 [object Int8Array] 4748494a4b4c4d4e
1 6 0 [object Int8Array] 48494a4b4c4d4e4f5051
1 6 1 [object Int8Array] 48494a4b4c4d4e
1 6 2 [object Int8Array] 48494a4b4c4d4e4f50
1 6 3 [object Int8Array] 
1 6 4 [object Int8Array] 
1 6 5 [object Int8Array] 
1 6 6 [object Int8Array] 
1 6 7 [object Int8Array] 48494a4b4c4d4e
1 7 0 [object Int8Array] 4f5051
1 7 1 [object Int8Array] 
1 7 2 [object Int8Array] 4f50
1 7 3 [object Int8Array] 
1 7 4 [object Int8Array] 
1 7 5 [object Int8Array] 
1 7 6 [object Int8Array] 
1 7 7 [object Int8Array] 
2 0 0 [object Uint8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 0 1 [object Uint8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 0 2 [object Uint8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 0 3 [object Uint8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 0 4 [object Uint8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 0 5 [object Uint8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 0 6 [object Uint8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 0 7 [object Uint8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 1 0 [object Uint8Array] 5d5e5f
2 1 1 [object Uint8Array] 
2 1 2 [object Uint8Array] 5d5e
2 1 3 [object Uint8Array] 
2 1 4 [object Uint8Array] 
2 1 5 [object Uint8Array] 
2 1 6 [object Uint8Array] 
2 1 7 [object Uint8Array] 
2 2 0 [object Uint8Array] 5f
2 2 1 [object Uint8Array] 
2 2 2 [object Uint8Array] 
2 2 3 [object Uint8Array] 
2 2 4 [object Uint8Array] 
2 2 5 [object Uint8Array] 
2 2 6 [object Uint8Array] 
2 2 7 [object Uint8Array] 
2 3 0 [object Uint8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 3 1 [object Uint8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c
2 3 2 [object Uint8Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e
2 3 3 [object Uint8Array] 
2 3 4 [object Uint8Array] 40
2 3 5 [object Uint8Array] 4041
2 3 6 [object Uint8Array] 404142
2 3 7 [object Uint8Array] 40414243444546474849
2 4 0 [object Uint8Array] 4142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 4 1 [object Uint8Array] 4142434445464748494a4b4c4d4e4f505152535455565758595a5b5c
2 4 2 [object Uint8Array] 4142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e
2 4 3 [object Uint8Array] 
2 4 4 [object Uint8Array] 
2 4 5 [object Uint8Array] 41
2 4 6 [object Uint8Array] 4142
2 4 7 [object Uint8Array] 414243444546474849
2 5 0 [object Uint8Array] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 5 1 [object Uint8Array] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c
2 5 2 [object Uint8Array] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e
2 5 3 [object Uint8Array] 
2 5 4 [object Uint8Array] 
2 5 5 [object Uint8Array] 
2 5 6 [object Uint8Array] 42
2 5 7 [object Uint8Array] 4243444546474849
2 6 0 [object Uint8Array] 434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 6 1 [object Uint8Array] 434445464748494a4b4c4d4e4f505152535455565758595a5b5c
2 6 2 [object Uint8Array] 434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e
2 6 3 [object Uint8Array] 
2 6 4 [object Uint8Array] 
2 6 5 [object Uint8Array] 
2 6 6 [object Uint8Array] 
2 6 7 [object Uint8Array] 43444546474849
2 7 0 [object Uint8Array] 4a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
2 7 1 [object Uint8Array] 4a4b4c4d4e4f505152535455565758595a5b5c
2 7 2 [object Uint8Array] 4a4b4c4d4e4f505152535455565758595a5b5c5d5e
2 7 3 [object Uint8Array] 
2 7 4 [object Uint8Array] 
2 7 5 [object Uint8Array] 
2 7 6 [object Uint8Array] 
2 7 7 [object Uint8Array] 
3 0 0 [object Uint8Array] 45464748494a4b4c4d4e4f5051
3 0 1 [object Uint8Array] 45464748494a4b4c4d4e4f5051
3 0 2 [object Uint8Array] 45464748494a4b4c4d4e4f5051
3 0 3 [object Uint8Array] 45464748494a4b4c4d4e4f5051
3 0 4 [object Uint8Array] 45464748494a4b4c4d4e4f5051
3 0 5 [object Uint8Array] 45464748494a4b4c4d4e4f5051
3 0 6 [object Uint8Array] 45464748494a4b4c4d4e4f5051
3 0 7 [object Uint8Array] 45464748494a4b4c4d4e4f5051
3 1 0 [object Uint8Array] 4f5051
3 1 1 [object Uint8Array] 
3 1 2 [object Uint8Array] 4f50
3 1 3 [object Uint8Array] 
3 1 4 [object Uint8Array] 
3 1 5 [object Uint8Array] 
3 1 6 [object Uint8Array] 
3 1 7 [object Uint8Array] 
3 2 0 [object Uint8Array] 51
3 2 1 [object Uint8Array] 
3 2 2 [object Uint8Array] 
3 2 3 [object Uint8Array] 
3 2 4 [object Uint8Array] 
3 2 5 [object Uint8Array] 
3 2 6 [object Uint8Array] 
3 2 7 [object Uint8Array] 
3 3 0 [object Uint8Array] 45464748494a4b4c4d4e4f5051
3 3 1 [object Uint8Array] 45464748494a4b4c4d4e
3 3 2 [object Uint8Array] 45464748494a4b4c4d4e4f50
3 3 3 [object Uint8Array] 
3 3 4 [object Uint8Array] 45
3 3 5 [object Uint8Array] 4546
3 3 6 [object Uint8Array] 454647
3 3 7 [object Uint8Array] 45464748494a4b4c4d4e
3 4 0 [object Uint8Array] 464748494a4b4c4d4e4f5051
3 4 1 [object Uint8Array] 464748494a4b4c4d4e
3 4 2 [object Uint8Array] 464748494a4b4c4d4e4f50
3 4 3 [object Uint8Array] 
3 4 4 [object Uint8Array] 
3 4 5 [object Uint8Array] 46
3 4 6 [object Uint8Array] 4647
3 4 7 [object Uint8Array] 464748494a4b4c4d4e
3 5 0 [object Uint8Array] 4748494a4b4c4d4e4f5051
3 5 1 [object Uint8Array] 4748494a4b4c4d4e
3 5 2 [object Uint8Array] 4748494a4b4c4d4e4f50
3 5 3 [object Uint8Array] 
3 5 4 [object Uint8Array] 
3 5 5 [object Uint8Array] 
3 5 6 [object Uint8Array] 47
3 5 7 [object Uint8Array] 4748494a4b4c4d4e
3 6 0 [object Uint8Array] 48494a4b4c4d4e4f5051
3 6 1 [object Uint8Array] 48494a4b4c4d4e
3 6 2 [object Uint8Array] 48494a4b4c4d4e4f50
3 6 3 [object Uint8Array] 
3 6 4 [object Uint8Array] 
3 6 5 [object Uint8Array] 
3 6 6 [object Uint8Array] 
3 6 7 [object Uint8Array] 48494a4b4c4d4e
3 7 0 [object Uint8Array] 4f5051
3 7 1 [object Uint8Array] 
3 7 2 [object Uint8Array] 4f50
3 7 3 [object Uint8Array] 
3 7 4 [object Uint8Array] 
3 7 5 [object Uint8Array] 
3 7 6 [object Uint8Array] 
3 7 7 [object Uint8Array] 
4 0 0 [object Uint8ClampedArray] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 0 1 [object Uint8ClampedArray] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 0 2 [object Uint8ClampedArray] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 0 3 [object Uint8ClampedArray] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 0 4 [object Uint8ClampedArray] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 0 5 [object Uint8ClampedArray] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 0 6 [object Uint8ClampedArray] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 0 7 [object Uint8ClampedArray] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 1 0 [object Uint8ClampedArray] 5d5e5f
4 1 1 [object Uint8ClampedArray] 
4 1 2 [object Uint8ClampedArray] 5d5e
4 1 3 [object Uint8ClampedArray] 
4 1 4 [object Uint8ClampedArray] 
4 1 5 [object Uint8ClampedArray] 
4 1 6 [object Uint8ClampedArray] 
4 1 7 [object Uint8ClampedArray] 
4 2 0 [object Uint8ClampedArray] 5f
4 2 1 [object Uint8ClampedArray] 
4 2 2 [object Uint8ClampedArray] 
4 2 3 [object Uint8ClampedArray] 
4 2 4 [object Uint8ClampedArray] 
4 2 5 [object Uint8ClampedArray] 
4 2 6 [object Uint8ClampedArray] 
4 2 7 [object Uint8ClampedArray] 
4 3 0 [object Uint8ClampedArray] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 3 1 [object Uint8ClampedArray] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c
4 3 2 [object Uint8ClampedArray] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e
4 3 3 [object Uint8ClampedArray] 
4 3 4 [object Uint8ClampedArray] 40
4 3 5 [object Uint8ClampedArray] 4041
4 3 6 [object Uint8ClampedArray] 404142
4 3 7 [object Uint8ClampedArray] 40414243444546474849
4 4 0 [object Uint8ClampedArray] 4142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 4 1 [object Uint8ClampedArray] 4142434445464748494a4b4c4d4e4f505152535455565758595a5b5c
4 4 2 [object Uint8ClampedArray] 4142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e
4 4 3 [object Uint8ClampedArray] 
4 4 4 [object Uint8ClampedArray] 
4 4 5 [object Uint8ClampedArray] 41
4 4 6 [object Uint8ClampedArray] 4142
4 4 7 [object Uint8ClampedArray] 414243444546474849
4 5 0 [object Uint8ClampedArray] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 5 1 [object Uint8ClampedArray] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c
4 5 2 [object Uint8ClampedArray] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e
4 5 3 [object Uint8ClampedArray] 
4 5 4 [object Uint8ClampedArray] 
4 5 5 [object Uint8ClampedArray] 
4 5 6 [object Uint8ClampedArray] 42
4 5 7 [object Uint8ClampedArray] 4243444546474849
4 6 0 [object Uint8ClampedArray] 434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 6 1 [object Uint8ClampedArray] 434445464748494a4b4c4d4e4f505152535455565758595a5b5c
4 6 2 [object Uint8ClampedArray] 434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e
4 6 3 [object Uint8ClampedArray] 
4 6 4 [object Uint8ClampedArray] 
4 6 5 [object Uint8ClampedArray] 
4 6 6 [object Uint8ClampedArray] 
4 6 7 [object Uint8ClampedArray] 43444546474849
4 7 0 [object Uint8ClampedArray] 4a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
4 7 1 [object Uint8ClampedArray] 4a4b4c4d4e4f505152535455565758595a5b5c
4 7 2 [object Uint8ClampedArray] 4a4b4c4d4e4f505152535455565758595a5b5c5d5e
4 7 3 [object Uint8ClampedArray] 
4 7 4 [object Uint8ClampedArray] 
4 7 5 [object Uint8ClampedArray] 
4 7 6 [object Uint8ClampedArray] 
4 7 7 [object Uint8ClampedArray] 
5 0 0 [object Uint8ClampedArray] 45464748494a4b4c4d4e4f5051
5 0 1 [object Uint8ClampedArray] 45464748494a4b4c4d4e4f5051
5 0 2 [object Uint8ClampedArray] 45464748494a4b4c4d4e4f5051
5 0 3 [object Uint8ClampedArray] 45464748494a4b4c4d4e4f5051
5 0 4 [object Uint8ClampedArray] 45464748494a4b4c4d4e4f5051
5 0 5 [object Uint8ClampedArray] 45464748494a4b4c4d4e4f5051
5 0 6 [object Uint8ClampedArray] 45464748494a4b4c4d4e4f5051
5 0 7 [object Uint8ClampedArray] 45464748494a4b4c4d4e4f5051
5 1 0 [object Uint8ClampedArray] 4f5051
5 1 1 [object Uint8ClampedArray] 
5 1 2 [object Uint8ClampedArray] 4f50
5 1 3 [object Uint8ClampedArray] 
5 1 4 [object Uint8ClampedArray] 
5 1 5 [object Uint8ClampedArray] 
5 1 6 [object Uint8ClampedArray] 
5 1 7 [object Uint8ClampedArray] 
5 2 0 [object Uint8ClampedArray] 51
5 2 1 [object Uint8ClampedArray] 
5 2 2 [object Uint8ClampedArray] 
5 2 3 [object Uint8ClampedArray] 
5 2 4 [object Uint8ClampedArray] 
5 2 5 [object Uint8ClampedArray] 
5 2 6 [object Uint8ClampedArray] 
5 2 7 [object Uint8ClampedArray] 
5 3 0 [object Uint8ClampedArray] 45464748494a4b4c4d4e4f5051
5 3 1 [object Uint8ClampedArray] 45464748494a4b4c4d4e
5 3 2 [object Uint8ClampedArray] 45464748494a4b4c4d4e4f50
5 3 3 [object Uint8ClampedArray] 
5 3 4 [object Uint8ClampedArray] 45
5 3 5 [object Uint8ClampedArray] 4546
5 3 6 [object Uint8ClampedArray] 454647
5 3 7 [object Uint8ClampedArray] 45464748494a4b4c4d4e
5 4 0 [object Uint8ClampedArray] 464748494a4b4c4d4e4f5051
5 4 1 [object Uint8ClampedArray] 464748494a4b4c4d4e
5 4 2 [object Uint8ClampedArray] 464748494a4b4c4d4e4f50
5 4 3 [object Uint8ClampedArray] 
5 4 4 [object Uint8ClampedArray] 
5 4 5 [object Uint8ClampedArray] 46
5 4 6 [object Uint8ClampedArray] 4647
5 4 7 [object Uint8ClampedArray] 464748494a4b4c4d4e
5 5 0 [object Uint8ClampedArray] 4748494a4b4c4d4e4f5051
5 5 1 [object Uint8ClampedArray] 4748494a4b4c4d4e
5 5 2 [object Uint8ClampedArray] 4748494a4b4c4d4e4f50
5 5 3 [object Uint8ClampedArray] 
5 5 4 [object Uint8ClampedArray] 
5 5 5 [object Uint8ClampedArray] 
5 5 6 [object Uint8ClampedArray] 47
5 5 7 [object Uint8ClampedArray] 4748494a4b4c4d4e
5 6 0 [object Uint8ClampedArray] 48494a4b4c4d4e4f5051
5 6 1 [object Uint8ClampedArray] 48494a4b4c4d4e
5 6 2 [object Uint8ClampedArray] 48494a4b4c4d4e4f50
5 6 3 [object Uint8ClampedArray] 
5 6 4 [object Uint8ClampedArray] 
5 6 5 [object Uint8ClampedArray] 
5 6 6 [object Uint8ClampedArray] 
5 6 7 [object Uint8ClampedArray] 48494a4b4c4d4e
5 7 0 [object Uint8ClampedArray] 4f5051
5 7 1 [object Uint8ClampedArray] 
5 7 2 [object Uint8ClampedArray] 4f50
5 7 3 [object Uint8ClampedArray] 
5 7 4 [object Uint8ClampedArray] 
5 7 5 [object Uint8ClampedArray] 
5 7 6 [object Uint8ClampedArray] 
5 7 7 [object Uint8ClampedArray] 
6 0 0 [object Int16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
6 0 1 [object Int16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
6 0 2 [object Int16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
6 0 3 [object Int16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
6 0 4 [object Int16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
6 0 5 [object Int16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
6 0 6 [object Int16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
6 0 7 [object Int16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
6 1 0 [object Int16Array] 5a5b5c5d5e5f
6 1 1 [object Int16Array] 
6 1 2 [object Int16Array] 5a5b5c5d
6 1 3 [object Int16Array] 
6 1 4 [object Int16Array] 
6 1 5 [object Int16Array] 
6 1 6 [object Int16Array] 
6 1 7 [object Int16Array] 
6 2 0 [object Int16Array] 5e5f
6 2 1 [object Int16Array] 
6 2 2 [object Int16Array] 
6 2 3 [object Int16Array] 
6 2 4 [object Int16Array] 
6 2 5 [object Int16Array] 
6 2 6 [object Int16Array] 
6 2 7 [object Int16Array] 
6 3 0 [object Int16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
6 3 1 [object Int16Array] 404142434445464748494a4b4c4d4e4f50515253545556575859
6 3 2 [object Int16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d
6 3 3 [object Int16Array] 
6 3 4 [object Int16Array] 4041
6 3 5 [object Int16Array] 40414243
6 3 6 [object Int16Array] 404142434445
6 3 7 [object Int16Array] 404142434445464748494a4b4c4d4e4f50515253
6 4 0 [object Int16Array] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
6 4 1 [object Int16Array] 42434445464748494a4b4c4d4e4f50515253545556575859
6 4 2 [object Int16Array] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d
6 4 3 [object Int16Array] 
6 4 4 [object Int16Array] 
6 4 5 [object Int16Array] 4243
6 4 6 [object Int16Array] 42434445
6 4 7 [object Int16Array] 42434445464748494a4b4c4d4e4f50515253
6 5 0 [object Int16Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
6 5 1 [object Int16Array] 4445464748494a4b4c4d4e4f50515253545556575859
6 5 2 [object Int16Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b5c5d
6 5 3 [object Int16Array] 
6 5 4 [object Int16Array] 
6 5 5 [object Int16Array] 
6 5 6 [object Int16Array] 4445
6 5 7 [object Int16Array] 4445464748494a4b4c4d4e4f50515253
6 6 0 [object Int16Array] 464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
6 6 1 [object Int16Array] 464748494a4b4c4d4e4f50515253545556575859
6 6 2 [object Int16Array] 464748494a4b4c4d4e4f505152535455565758595a5b5c5d
6 6 3 [object Int16Array] 
6 6 4 [object Int16Array] 
6 6 5 [object Int16Array] 
6 6 6 [object Int16Array] 
6 6 7 [object Int16Array] 464748494a4b4c4d4e4f50515253
6 7 0 [object Int16Array] 5455565758595a5b5c5d5e5f
6 7 1 [object Int16Array] 545556575859
6 7 2 [object Int16Array] 5455565758595a5b5c5d
6 7 3 [object Int16Array] 
6 7 4 [object Int16Array] 
6 7 5 [object Int16Array] 
6 7 6 [object Int16Array] 
6 7 7 [object Int16Array] 
7 0 0 [object Int16Array] 4445464748494a4b4c4d4e4f50515253
7 0 1 [object Int16Array] 4445464748494a4b4c4d4e4f50515253
7 0 2 [object Int16Array] 4445464748494a4b4c4d4e4f50515253
7 0 3 [object Int16Array] 4445464748494a4b4c4d4e4f50515253
7 0 4 [object Int16Array] 4445464748494a4b4c4d4e4f50515253
7 0 5 [object Int16Array] 4445464748494a4b4c4d4e4f50515253
7 0 6 [object Int16Array] 4445464748494a4b4c4d4e4f50515253
7 0 7 [object Int16Array] 4445464748494a4b4c4d4e4f50515253
7 1 0 [object Int16Array] 4e4f50515253
7 1 1 [object Int16Array] 
7 1 2 [object Int16Array] 4e4f5051
7 1 3 [object Int16Array] 
7 1 4 [object Int16Array] 
7 1 5 [object Int16Array] 
7 1 6 [object Int16Array] 
7 1 7 [object Int16Array] 4e4f50515253
7 2 0 [object Int16Array] 5253
7 2 1 [object Int16Array] 
7 2 2 [object Int16Array] 
7 2 3 [object Int16Array] 
7 2 4 [object Int16Array] 
7 2 5 [object Int16Array] 
7 2 6 [object Int16Array] 
7 2 7 [object Int16Array] 5253
7 3 0 [object Int16Array] 4445464748494a4b4c4d4e4f50515253
7 3 1 [object Int16Array] 4445464748494a4b4c4d
7 3 2 [object Int16Array] 4445464748494a4b4c4d4e4f5051
7 3 3 [object Int16Array] 
7 3 4 [object Int16Array] 4445
7 3 5 [object Int16Array] 44454647
7 3 6 [object Int16Array] 444546474849
7 3 7 [object Int16Array] 4445464748494a4b4c4d4e4f50515253
7 4 0 [object Int16Array] 464748494a4b4c4d4e4f50515253
7 4 1 [object Int16Array] 464748494a4b4c4d
7 4 2 [object Int16Array] 464748494a4b4c4d4e4f5051
7 4 3 [object Int16Array] 
7 4 4 [object Int16Array] 
7 4 5 [object Int16Array] 4647
7 4 6 [object Int16Array] 46474849
7 4 7 [object Int16Array] 464748494a4b4c4d4e4f50515253
7 5 0 [object Int16Array] 48494a4b4c4d4e4f50515253
7 5 1 [object Int16Array] 48494a4b4c4d
7 5 2 [object Int16Array] 48494a4b4c4d4e4f5051
7 5 3 [object Int16Array] 
7 5 4 [object Int16Array] 
7 5 5 [object Int16Array] 
7 5 6 [object Int16Array] 4849
7 5 7 [object Int16Array] 48494a4b4c4d4e4f50515253
7 6 0 [object Int16Array] 4a4b4c4d4e4f50515253
7 6 1 [object Int16Array] 4a4b4c4d
7 6 2 [object Int16Array] 4a4b4c4d4e4f5051
7 6 3 [object Int16Array] 
7 6 4 [object Int16Array] 
7 6 5 [object Int16Array] 
7 6 6 [object Int16Array] 
7 6 7 [object Int16Array] 4a4b4c4d4e4f50515253
7 7 0 [object Int16Array] 
7 7 1 [object Int16Array] 
7 7 2 [object Int16Array] 
7 7 3 [object Int16Array] 
7 7 4 [object Int16Array] 
7 7 5 [object Int16Array] 
7 7 6 [object Int16Array] 
7 7 7 [object Int16Array] 
8 0 0 [object Uint16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
8 0 1 [object Uint16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
8 0 2 [object Uint16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
8 0 3 [object Uint16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
8 0 4 [object Uint16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
8 0 5 [object Uint16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
8 0 6 [object Uint16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
8 0 7 [object Uint16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
8 1 0 [object Uint16Array] 5a5b5c5d5e5f
8 1 1 [object Uint16Array] 
8 1 2 [object Uint16Array] 5a5b5c5d
8 1 3 [object Uint16Array] 
8 1 4 [object Uint16Array] 
8 1 5 [object Uint16Array] 
8 1 6 [object Uint16Array] 
8 1 7 [object Uint16Array] 
8 2 0 [object Uint16Array] 5e5f
8 2 1 [object Uint16Array] 
8 2 2 [object Uint16Array] 
8 2 3 [object Uint16Array] 
8 2 4 [object Uint16Array] 
8 2 5 [object Uint16Array] 
8 2 6 [object Uint16Array] 
8 2 7 [object Uint16Array] 
8 3 0 [object Uint16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
8 3 1 [object Uint16Array] 404142434445464748494a4b4c4d4e4f50515253545556575859
8 3 2 [object Uint16Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d
8 3 3 [object Uint16Array] 
8 3 4 [object Uint16Array] 4041
8 3 5 [object Uint16Array] 40414243
8 3 6 [object Uint16Array] 404142434445
8 3 7 [object Uint16Array] 404142434445464748494a4b4c4d4e4f50515253
8 4 0 [object Uint16Array] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
8 4 1 [object Uint16Array] 42434445464748494a4b4c4d4e4f50515253545556575859
8 4 2 [object Uint16Array] 42434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d
8 4 3 [object Uint16Array] 
8 4 4 [object Uint16Array] 
8 4 5 [object Uint16Array] 4243
8 4 6 [object Uint16Array] 42434445
8 4 7 [object Uint16Array] 42434445464748494a4b4c4d4e4f50515253
8 5 0 [object Uint16Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
8 5 1 [object Uint16Array] 4445464748494a4b4c4d4e4f50515253545556575859
8 5 2 [object Uint16Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b5c5d
8 5 3 [object Uint16Array] 
8 5 4 [object Uint16Array] 
8 5 5 [object Uint16Array] 
8 5 6 [object Uint16Array] 4445
8 5 7 [object Uint16Array] 4445464748494a4b4c4d4e4f50515253
8 6 0 [object Uint16Array] 464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
8 6 1 [object Uint16Array] 464748494a4b4c4d4e4f50515253545556575859
8 6 2 [object Uint16Array] 464748494a4b4c4d4e4f505152535455565758595a5b5c5d
8 6 3 [object Uint16Array] 
8 6 4 [object Uint16Array] 
8 6 5 [object Uint16Array] 
8 6 6 [object Uint16Array] 
8 6 7 [object Uint16Array] 464748494a4b4c4d4e4f50515253
8 7 0 [object Uint16Array] 5455565758595a5b5c5d5e5f
8 7 1 [object Uint16Array] 545556575859
8 7 2 [object Uint16Array] 5455565758595a5b5c5d
8 7 3 [object Uint16Array] 
8 7 4 [object Uint16Array] 
8 7 5 [object Uint16Array] 
8 7 6 [object Uint16Array] 
8 7 7 [object Uint16Array] 
9 0 0 [object Uint16Array] 4c4d4e4f505152535455565758595a5b
9 0 1 [object Uint16Array] 4c4d4e4f505152535455565758595a5b
9 0 2 [object Uint16Array] 4c4d4e4f505152535455565758595a5b
9 0 3 [object Uint16Array] 4c4d4e4f505152535455565758595a5b
9 0 4 [object Uint16Array] 4c4d4e4f505152535455565758595a5b
9 0 5 [object Uint16Array] 4c4d4e4f505152535455565758595a5b
9 0 6 [object Uint16Array] 4c4d4e4f505152535455565758595a5b
9 0 7 [object Uint16Array] 4c4d4e4f505152535455565758595a5b
9 1 0 [object Uint16Array] 565758595a5b
9 1 1 [object Uint16Array] 
9 1 2 [object Uint16Array] 56575859
9 1 3 [object Uint16Array] 
9 1 4 [object Uint16Array] 
9 1 5 [object Uint16Array] 
9 1 6 [object Uint16Array] 
9 1 7 [object Uint16Array] 565758595a5b
9 2 0 [object Uint16Array] 5a5b
9 2 1 [object Uint16Array] 
9 2 2 [object Uint16Array] 
9 2 3 [object Uint16Array] 
9 2 4 [object Uint16Array] 
9 2 5 [object Uint16Array] 
9 2 6 [object Uint16Array] 
9 2 7 [object Uint16Array] 5a5b
9 3 0 [object Uint16Array] 4c4d4e4f505152535455565758595a5b
9 3 1 [object Uint16Array] 4c4d4e4f505152535455
9 3 2 [object Uint16Array] 4c4d4e4f50515253545556575859
9 3 3 [object Uint16Array] 
9 3 4 [object Uint16Array] 4c4d
9 3 5 [object Uint16Array] 4c4d4e4f
9 3 6 [object Uint16Array] 4c4d4e4f5051
9 3 7 [object Uint16Array] 4c4d4e4f505152535455565758595a5b
9 4 0 [object Uint16Array] 4e4f505152535455565758595a5b
9 4 1 [object Uint16Array] 4e4f505152535455
9 4 2 [object Uint16Array] 4e4f50515253545556575859
9 4 3 [object Uint16Array] 
9 4 4 [object Uint16Array] 
9 4 5 [object Uint16Array] 4e4f
9 4 6 [object Uint16Array] 4e4f5051
9 4 7 [object Uint16Array] 4e4f505152535455565758595a5b
9 5 0 [object Uint16Array] 505152535455565758595a5b
9 5 1 [object Uint16Array] 505152535455
9 5 2 [object Uint16Array] 50515253545556575859
9 5 3 [object Uint16Array] 
9 5 4 [object Uint16Array] 
9 5 5 [object Uint16Array] 
9 5 6 [object Uint16Array] 5051
9 5 7 [object Uint16Array] 505152535455565758595a5b
9 6 0 [object Uint16Array] 52535455565758595a5b
9 6 1 [object Uint16Array] 52535455
9 6 2 [object Uint16Array] 5253545556575859
9 6 3 [object Uint16Array] 
9 6 4 [object Uint16Array] 
9 6 5 [object Uint16Array] 
9 6 6 [object Uint16Array] 
9 6 7 [object Uint16Array] 52535455565758595a5b
9 7 0 [object Uint16Array] 
9 7 1 [object Uint16Array] 
9 7 2 [object Uint16Array] 
9 7 3 [object Uint16Array] 
9 7 4 [object Uint16Array] 
9 7 5 [object Uint16Array] 
9 7 6 [object Uint16Array] 
9 7 7 [object Uint16Array] 
10 0 0 [object Int32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 0 1 [object Int32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 0 2 [object Int32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 0 3 [object Int32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 0 4 [object Int32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 0 5 [object Int32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 0 6 [object Int32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 0 7 [object Int32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 1 0 [object Int32Array] 5455565758595a5b5c5d5e5f
10 1 1 [object Int32Array] 
10 1 2 [object Int32Array] 5455565758595a5b
10 1 3 [object Int32Array] 
10 1 4 [object Int32Array] 
10 1 5 [object Int32Array] 
10 1 6 [object Int32Array] 
10 1 7 [object Int32Array] 5455565758595a5b5c5d5e5f
10 2 0 [object Int32Array] 5c5d5e5f
10 2 1 [object Int32Array] 
10 2 2 [object Int32Array] 
10 2 3 [object Int32Array] 
10 2 4 [object Int32Array] 
10 2 5 [object Int32Array] 
10 2 6 [object Int32Array] 
10 2 7 [object Int32Array] 5c5d5e5f
10 3 0 [object Int32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 3 1 [object Int32Array] 404142434445464748494a4b4c4d4e4f50515253
10 3 2 [object Int32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b
10 3 3 [object Int32Array] 
10 3 4 [object Int32Array] 40414243
10 3 5 [object Int32Array] 4041424344454647
10 3 6 [object Int32Array] 404142434445464748494a4b
10 3 7 [object Int32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 4 0 [object Int32Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 4 1 [object Int32Array] 4445464748494a4b4c4d4e4f50515253
10 4 2 [object Int32Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b
10 4 3 [object Int32Array] 
10 4 4 [object Int32Array] 
10 4 5 [object Int32Array] 44454647
10 4 6 [object Int32Array] 4445464748494a4b
10 4 7 [object Int32Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 5 0 [object Int32Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 5 1 [object Int32Array] 48494a4b4c4d4e4f50515253
10 5 2 [object Int32Array] 48494a4b4c4d4e4f505152535455565758595a5b
10 5 3 [object Int32Array] 
10 5 4 [object Int32Array] 
10 5 5 [object Int32Array] 
10 5 6 [object Int32Array] 48494a4b
10 5 7 [object Int32Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
10 6 0 [object Int32Array] 4c4d4e4f505152535455565758595a5b5c5d5e5f
10 6 1 [object Int32Array] 4c4d4e4f50515253
10 6 2 [object Int32Array] 4c4d4e4f505152535455565758595a5b
10 6 3 [object Int32Array] 
10 6 4 [object Int32Array] 
10 6 5 [object Int32Array] 
10 6 6 [object Int32Array] 
10 6 7 [object Int32Array] 4c4d4e4f505152535455565758595a5b5c5d5e5f
10 7 0 [object Int32Array] 
10 7 1 [object Int32Array] 
10 7 2 [object Int32Array] 
10 7 3 [object Int32Array] 
10 7 4 [object Int32Array] 
10 7 5 [object Int32Array] 
10 7 6 [object Int32Array] 
10 7 7 [object Int32Array] 
11 0 0 [object Int32Array] 4445464748494a4b4c4d4e4f50515253
11 0 1 [object Int32Array] 4445464748494a4b4c4d4e4f50515253
11 0 2 [object Int32Array] 4445464748494a4b4c4d4e4f50515253
11 0 3 [object Int32Array] 4445464748494a4b4c4d4e4f50515253
11 0 4 [object Int32Array] 4445464748494a4b4c4d4e4f50515253
11 0 5 [object Int32Array] 4445464748494a4b4c4d4e4f50515253
11 0 6 [object Int32Array] 4445464748494a4b4c4d4e4f50515253
11 0 7 [object Int32Array] 4445464748494a4b4c4d4e4f50515253
11 1 0 [object Int32Array] 48494a4b4c4d4e4f50515253
11 1 1 [object Int32Array] 
11 1 2 [object Int32Array] 48494a4b4c4d4e4f
11 1 3 [object Int32Array] 
11 1 4 [object Int32Array] 
11 1 5 [object Int32Array] 48494a4b
11 1 6 [object Int32Array] 48494a4b4c4d4e4f
11 1 7 [object Int32Array] 48494a4b4c4d4e4f50515253
11 2 0 [object Int32Array] 50515253
11 2 1 [object Int32Array] 
11 2 2 [object Int32Array] 
11 2 3 [object Int32Array] 
11 2 4 [object Int32Array] 
11 2 5 [object Int32Array] 
11 2 6 [object Int32Array] 
11 2 7 [object Int32Array] 50515253
11 3 0 [object Int32Array] 4445464748494a4b4c4d4e4f50515253
11 3 1 [object Int32Array] 44454647
11 3 2 [object Int32Array] 4445464748494a4b4c4d4e4f
11 3 3 [object Int32Array] 
11 3 4 [object Int32Array] 44454647
11 3 5 [object Int32Array] 4445464748494a4b
11 3 6 [object Int32Array] 4445464748494a4b4c4d4e4f
11 3 7 [object Int32Array] 4445464748494a4b4c4d4e4f50515253
11 4 0 [object Int32Array] 48494a4b4c4d4e4f50515253
11 4 1 [object Int32Array] 
11 4 2 [object Int32Array] 48494a4b4c4d4e4f
11 4 3 [object Int32Array] 
11 4 4 [object Int32Array] 
11 4 5 [object Int32Array] 48494a4b
11 4 6 [object Int32Array] 48494a4b4c4d4e4f
11 4 7 [object Int32Array] 48494a4b4c4d4e4f50515253
11 5 0 [object Int32Array] 4c4d4e4f50515253
11 5 1 [object Int32Array] 
11 5 2 [object Int32Array] 4c4d4e4f
11 5 3 [object Int32Array] 
11 5 4 [object Int32Array] 
11 5 5 [object Int32Array] 
11 5 6 [object Int32Array] 4c4d4e4f
11 5 7 [object Int32Array] 4c4d4e4f50515253
11 6 0 [object Int32Array] 50515253
11 6 1 [object Int32Array] 
11 6 2 [object Int32Array] 
11 6 3 [object Int32Array] 
11 6 4 [object Int32Array] 
11 6 5 [object Int32Array] 
11 6 6 [object Int32Array] 
11 6 7 [object Int32Array] 50515253
11 7 0 [object Int32Array] 
11 7 1 [object Int32Array] 
11 7 2 [object Int32Array] 
11 7 3 [object Int32Array] 
11 7 4 [object Int32Array] 
11 7 5 [object Int32Array] 
11 7 6 [object Int32Array] 
11 7 7 [object Int32Array] 
12 0 0 [object Uint32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 0 1 [object Uint32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 0 2 [object Uint32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 0 3 [object Uint32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 0 4 [object Uint32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 0 5 [object Uint32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 0 6 [object Uint32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 0 7 [object Uint32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 1 0 [object Uint32Array] 5455565758595a5b5c5d5e5f
12 1 1 [object Uint32Array] 
12 1 2 [object Uint32Array] 5455565758595a5b
12 1 3 [object Uint32Array] 
12 1 4 [object Uint32Array] 
12 1 5 [object Uint32Array] 
12 1 6 [object Uint32Array] 
12 1 7 [object Uint32Array] 5455565758595a5b5c5d5e5f
12 2 0 [object Uint32Array] 5c5d5e5f
12 2 1 [object Uint32Array] 
12 2 2 [object Uint32Array] 
12 2 3 [object Uint32Array] 
12 2 4 [object Uint32Array] 
12 2 5 [object Uint32Array] 
12 2 6 [object Uint32Array] 
12 2 7 [object Uint32Array] 5c5d5e5f
12 3 0 [object Uint32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 3 1 [object Uint32Array] 404142434445464748494a4b4c4d4e4f50515253
12 3 2 [object Uint32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b
12 3 3 [object Uint32Array] 
12 3 4 [object Uint32Array] 40414243
12 3 5 [object Uint32Array] 4041424344454647
12 3 6 [object Uint32Array] 404142434445464748494a4b
12 3 7 [object Uint32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 4 0 [object Uint32Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 4 1 [object Uint32Array] 4445464748494a4b4c4d4e4f50515253
12 4 2 [object Uint32Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b
12 4 3 [object Uint32Array] 
12 4 4 [object Uint32Array] 
12 4 5 [object Uint32Array] 44454647
12 4 6 [object Uint32Array] 4445464748494a4b
12 4 7 [object Uint32Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 5 0 [object Uint32Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 5 1 [object Uint32Array] 48494a4b4c4d4e4f50515253
12 5 2 [object Uint32Array] 48494a4b4c4d4e4f505152535455565758595a5b
12 5 3 [object Uint32Array] 
12 5 4 [object Uint32Array] 
12 5 5 [object Uint32Array] 
12 5 6 [object Uint32Array] 48494a4b
12 5 7 [object Uint32Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
12 6 0 [object Uint32Array] 4c4d4e4f505152535455565758595a5b5c5d5e5f
12 6 1 [object Uint32Array] 4c4d4e4f50515253
12 6 2 [object Uint32Array] 4c4d4e4f505152535455565758595a5b
12 6 3 [object Uint32Array] 
12 6 4 [object Uint32Array] 
12 6 5 [object Uint32Array] 
12 6 6 [object Uint32Array] 
12 6 7 [object Uint32Array] 4c4d4e4f505152535455565758595a5b5c5d5e5f
12 7 0 [object Uint32Array] 
12 7 1 [object Uint32Array] 
12 7 2 [object Uint32Array] 
12 7 3 [object Uint32Array] 
12 7 4 [object Uint32Array] 
12 7 5 [object Uint32Array] 
12 7 6 [object Uint32Array] 
12 7 7 [object Uint32Array] 
13 0 0 [object Uint32Array] 4c4d4e4f505152535455565758595a5b
13 0 1 [object Uint32Array] 4c4d4e4f505152535455565758595a5b
13 0 2 [object Uint32Array] 4c4d4e4f505152535455565758595a5b
13 0 3 [object Uint32Array] 4c4d4e4f505152535455565758595a5b
13 0 4 [object Uint32Array] 4c4d4e4f505152535455565758595a5b
13 0 5 [object Uint32Array] 4c4d4e4f505152535455565758595a5b
13 0 6 [object Uint32Array] 4c4d4e4f505152535455565758595a5b
13 0 7 [object Uint32Array] 4c4d4e4f505152535455565758595a5b
13 1 0 [object Uint32Array] 505152535455565758595a5b
13 1 1 [object Uint32Array] 
13 1 2 [object Uint32Array] 5051525354555657
13 1 3 [object Uint32Array] 
13 1 4 [object Uint32Array] 
13 1 5 [object Uint32Array] 50515253
13 1 6 [object Uint32Array] 5051525354555657
13 1 7 [object Uint32Array] 505152535455565758595a5b
13 2 0 [object Uint32Array] 58595a5b
13 2 1 [object Uint32Array] 
13 2 2 [object Uint32Array] 
13 2 3 [object Uint32Array] 
13 2 4 [object Uint32Array] 
13 2 5 [object Uint32Array] 
13 2 6 [object Uint32Array] 
13 2 7 [object Uint32Array] 58595a5b
13 3 0 [object Uint32Array] 4c4d4e4f505152535455565758595a5b
13 3 1 [object Uint32Array] 4c4d4e4f
13 3 2 [object Uint32Array] 4c4d4e4f5051525354555657
13 3 3 [object Uint32Array] 
13 3 4 [object Uint32Array] 4c4d4e4f
13 3 5 [object Uint32Array] 4c4d4e4f50515253
13 3 6 [object Uint32Array] 4c4d4e4f5051525354555657
13 3 7 [object Uint32Array] 4c4d4e4f505152535455565758595a5b
13 4 0 [object Uint32Array] 505152535455565758595a5b
13 4 1 [object Uint32Array] 
13 4 2 [object Uint32Array] 5051525354555657
13 4 3 [object Uint32Array] 
13 4 4 [object Uint32Array] 
13 4 5 [object Uint32Array] 50515253
13 4 6 [object Uint32Array] 5051525354555657
13 4 7 [object Uint32Array] 505152535455565758595a5b
13 5 0 [object Uint32Array] 5455565758595a5b
13 5 1 [object Uint32Array] 
13 5 2 [object Uint32Array] 54555657
13 5 3 [object Uint32Array] 
13 5 4 [object Uint32Array] 
13 5 5 [object Uint32Array] 
13 5 6 [object Uint32Array] 54555657
13 5 7 [object Uint32Array] 5455565758595a5b
13 6 0 [object Uint32Array] 58595a5b
13 6 1 [object Uint32Array] 
13 6 2 [object Uint32Array] 
13 6 3 [object Uint32Array] 
13 6 4 [object Uint32Array] 
13 6 5 [object Uint32Array] 
13 6 6 [object Uint32Array] 
13 6 7 [object Uint32Array] 58595a5b
13 7 0 [object Uint32Array] 
13 7 1 [object Uint32Array] 
13 7 2 [object Uint32Array] 
13 7 3 [object Uint32Array] 
13 7 4 [object Uint32Array] 
13 7 5 [object Uint32Array] 
13 7 6 [object Uint32Array] 
13 7 7 [object Uint32Array] 
14 0 0 [object Float32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 0 1 [object Float32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 0 2 [object Float32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 0 3 [object Float32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 0 4 [object Float32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 0 5 [object Float32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 0 6 [object Float32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 0 7 [object Float32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 1 0 [object Float32Array] 5455565758595a5b5c5d5e5f
14 1 1 [object Float32Array] 
14 1 2 [object Float32Array] 5455565758595a5b
14 1 3 [object Float32Array] 
14 1 4 [object Float32Array] 
14 1 5 [object Float32Array] 
14 1 6 [object Float32Array] 
14 1 7 [object Float32Array] 5455565758595a5b5c5d5e5f
14 2 0 [object Float32Array] 5c5d5e5f
14 2 1 [object Float32Array] 
14 2 2 [object Float32Array] 
14 2 3 [object Float32Array] 
14 2 4 [object Float32Array] 
14 2 5 [object Float32Array] 
14 2 6 [object Float32Array] 
14 2 7 [object Float32Array] 5c5d5e5f
14 3 0 [object Float32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 3 1 [object Float32Array] 404142434445464748494a4b4c4d4e4f50515253
14 3 2 [object Float32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b
14 3 3 [object Float32Array] 
14 3 4 [object Float32Array] 40414243
14 3 5 [object Float32Array] 4041424344454647
14 3 6 [object Float32Array] 404142434445464748494a4b
14 3 7 [object Float32Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 4 0 [object Float32Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 4 1 [object Float32Array] 4445464748494a4b4c4d4e4f50515253
14 4 2 [object Float32Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b
14 4 3 [object Float32Array] 
14 4 4 [object Float32Array] 
14 4 5 [object Float32Array] 44454647
14 4 6 [object Float32Array] 4445464748494a4b
14 4 7 [object Float32Array] 4445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 5 0 [object Float32Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 5 1 [object Float32Array] 48494a4b4c4d4e4f50515253
14 5 2 [object Float32Array] 48494a4b4c4d4e4f505152535455565758595a5b
14 5 3 [object Float32Array] 
14 5 4 [object Float32Array] 
14 5 5 [object Float32Array] 
14 5 6 [object Float32Array] 48494a4b
14 5 7 [object Float32Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
14 6 0 [object Float32Array] 4c4d4e4f505152535455565758595a5b5c5d5e5f
14 6 1 [object Float32Array] 4c4d4e4f50515253
14 6 2 [object Float32Array] 4c4d4e4f505152535455565758595a5b
14 6 3 [object Float32Array] 
14 6 4 [object Float32Array] 
14 6 5 [object Float32Array] 
14 6 6 [object Float32Array] 
14 6 7 [object Float32Array] 4c4d4e4f505152535455565758595a5b5c5d5e5f
14 7 0 [object Float32Array] 
14 7 1 [object Float32Array] 
14 7 2 [object Float32Array] 
14 7 3 [object Float32Array] 
14 7 4 [object Float32Array] 
14 7 5 [object Float32Array] 
14 7 6 [object Float32Array] 
14 7 7 [object Float32Array] 
15 0 0 [object Float32Array] 4445464748494a4b4c4d4e4f50515253
15 0 1 [object Float32Array] 4445464748494a4b4c4d4e4f50515253
15 0 2 [object Float32Array] 4445464748494a4b4c4d4e4f50515253
15 0 3 [object Float32Array] 4445464748494a4b4c4d4e4f50515253
15 0 4 [object Float32Array] 4445464748494a4b4c4d4e4f50515253
15 0 5 [object Float32Array] 4445464748494a4b4c4d4e4f50515253
15 0 6 [object Float32Array] 4445464748494a4b4c4d4e4f50515253
15 0 7 [object Float32Array] 4445464748494a4b4c4d4e4f50515253
15 1 0 [object Float32Array] 48494a4b4c4d4e4f50515253
15 1 1 [object Float32Array] 
15 1 2 [object Float32Array] 48494a4b4c4d4e4f
15 1 3 [object Float32Array] 
15 1 4 [object Float32Array] 
15 1 5 [object Float32Array] 48494a4b
15 1 6 [object Float32Array] 48494a4b4c4d4e4f
15 1 7 [object Float32Array] 48494a4b4c4d4e4f50515253
15 2 0 [object Float32Array] 50515253
15 2 1 [object Float32Array] 
15 2 2 [object Float32Array] 
15 2 3 [object Float32Array] 
15 2 4 [object Float32Array] 
15 2 5 [object Float32Array] 
15 2 6 [object Float32Array] 
15 2 7 [object Float32Array] 50515253
15 3 0 [object Float32Array] 4445464748494a4b4c4d4e4f50515253
15 3 1 [object Float32Array] 44454647
15 3 2 [object Float32Array] 4445464748494a4b4c4d4e4f
15 3 3 [object Float32Array] 
15 3 4 [object Float32Array] 44454647
15 3 5 [object Float32Array] 4445464748494a4b
15 3 6 [object Float32Array] 4445464748494a4b4c4d4e4f
15 3 7 [object Float32Array] 4445464748494a4b4c4d4e4f50515253
15 4 0 [object Float32Array] 48494a4b4c4d4e4f50515253
15 4 1 [object Float32Array] 
15 4 2 [object Float32Array] 48494a4b4c4d4e4f
15 4 3 [object Float32Array] 
15 4 4 [object Float32Array] 
15 4 5 [object Float32Array] 48494a4b
15 4 6 [object Float32Array] 48494a4b4c4d4e4f
15 4 7 [object Float32Array] 48494a4b4c4d4e4f50515253
15 5 0 [object Float32Array] 4c4d4e4f50515253
15 5 1 [object Float32Array] 
15 5 2 [object Float32Array] 4c4d4e4f
15 5 3 [object Float32Array] 
15 5 4 [object Float32Array] 
15 5 5 [object Float32Array] 
15 5 6 [object Float32Array] 4c4d4e4f
15 5 7 [object Float32Array] 4c4d4e4f50515253
15 6 0 [object Float32Array] 50515253
15 6 1 [object Float32Array] 
15 6 2 [object Float32Array] 
15 6 3 [object Float32Array] 
15 6 4 [object Float32Array] 
15 6 5 [object Float32Array] 
15 6 6 [object Float32Array] 
15 6 7 [object Float32Array] 50515253
15 7 0 [object Float32Array] 
15 7 1 [object Float32Array] 
15 7 2 [object Float32Array] 
15 7 3 [object Float32Array] 
15 7 4 [object Float32Array] 
15 7 5 [object Float32Array] 
15 7 6 [object Float32Array] 
15 7 7 [object Float32Array] 
16 0 0 [object Float64Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 0 1 [object Float64Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 0 2 [object Float64Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 0 3 [object Float64Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 0 4 [object Float64Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 0 5 [object Float64Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 0 6 [object Float64Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 0 7 [object Float64Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 1 0 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 1 1 [object Float64Array] 
16 1 2 [object Float64Array] 48494a4b4c4d4e4f5051525354555657
16 1 3 [object Float64Array] 
16 1 4 [object Float64Array] 
16 1 5 [object Float64Array] 48494a4b4c4d4e4f
16 1 6 [object Float64Array] 48494a4b4c4d4e4f5051525354555657
16 1 7 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 2 0 [object Float64Array] 58595a5b5c5d5e5f
16 2 1 [object Float64Array] 
16 2 2 [object Float64Array] 
16 2 3 [object Float64Array] 
16 2 4 [object Float64Array] 
16 2 5 [object Float64Array] 
16 2 6 [object Float64Array] 
16 2 7 [object Float64Array] 58595a5b5c5d5e5f
16 3 0 [object Float64Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 3 1 [object Float64Array] 4041424344454647
16 3 2 [object Float64Array] 404142434445464748494a4b4c4d4e4f5051525354555657
16 3 3 [object Float64Array] 
16 3 4 [object Float64Array] 4041424344454647
16 3 5 [object Float64Array] 404142434445464748494a4b4c4d4e4f
16 3 6 [object Float64Array] 404142434445464748494a4b4c4d4e4f5051525354555657
16 3 7 [object Float64Array] 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 4 0 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 4 1 [object Float64Array] 
16 4 2 [object Float64Array] 48494a4b4c4d4e4f5051525354555657
16 4 3 [object Float64Array] 
16 4 4 [object Float64Array] 
16 4 5 [object Float64Array] 48494a4b4c4d4e4f
16 4 6 [object Float64Array] 48494a4b4c4d4e4f5051525354555657
16 4 7 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
16 5 0 [object Float64Array] 505152535455565758595a5b5c5d5e5f
16 5 1 [object Float64Array] 
16 5 2 [object Float64Array] 5051525354555657
16 5 3 [object Float64Array] 
16 5 4 [object Float64Array] 
16 5 5 [object Float64Array] 
16 5 6 [object Float64Array] 5051525354555657
16 5 7 [object Float64Array] 505152535455565758595a5b5c5d5e5f
16 6 0 [object Float64Array] 58595a5b5c5d5e5f
16 6 1 [object Float64Array] 
16 6 2 [object Float64Array] 
16 6 3 [object Float64Array] 
16 6 4 [object Float64Array] 
16 6 5 [object Float64Array] 
16 6 6 [object Float64Array] 
16 6 7 [object Float64Array] 58595a5b5c5d5e5f
16 7 0 [object Float64Array] 
16 7 1 [object Float64Array] 
16 7 2 [object Float64Array] 
16 7 3 [object Float64Array] 
16 7 4 [object Float64Array] 
16 7 5 [object Float64Array] 
16 7 6 [object Float64Array] 
16 7 7 [object Float64Array] 
17 0 0 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 0 1 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 0 2 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 0 3 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 0 4 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 0 5 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 0 6 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 0 7 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 1 0 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 1 1 [object Float64Array] 
17 1 2 [object Float64Array] 48494a4b4c4d4e4f5051525354555657
17 1 3 [object Float64Array] 
17 1 4 [object Float64Array] 48494a4b4c4d4e4f
17 1 5 [object Float64Array] 48494a4b4c4d4e4f5051525354555657
17 1 6 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 1 7 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 2 0 [object Float64Array] 58595a5b5c5d5e5f
17 2 1 [object Float64Array] 
17 2 2 [object Float64Array] 
17 2 3 [object Float64Array] 
17 2 4 [object Float64Array] 
17 2 5 [object Float64Array] 
17 2 6 [object Float64Array] 58595a5b5c5d5e5f
17 2 7 [object Float64Array] 58595a5b5c5d5e5f
17 3 0 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 3 1 [object Float64Array] 
17 3 2 [object Float64Array] 48494a4b4c4d4e4f5051525354555657
17 3 3 [object Float64Array] 
17 3 4 [object Float64Array] 48494a4b4c4d4e4f
17 3 5 [object Float64Array] 48494a4b4c4d4e4f5051525354555657
17 3 6 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 3 7 [object Float64Array] 48494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f
17 4 0 [object Float64Array] 505152535455565758595a5b5c5d5e5f
17 4 1 [object Float64Array] 
17 4 2 [object Float64Array] 5051525354555657
17 4 3 [object Float64Array] 
17 4 4 [object Float64Array] 
17 4 5 [object Float64Array] 5051525354555657
17 4 6 [object Float64Array] 505152535455565758595a5b5c5d5e5f
17 4 7 [object Float64Array] 505152535455565758595a5b5c5d5e5f
17 5 0 [object Float64Array] 58595a5b5c5d5e5f
17 5 1 [object Float64Array] 
17 5 2 [object Float64Array] 
17 5 3 [object Float64Array] 
17 5 4 [object Float64Array] 
17 5 5 [object Float64Array] 
17 5 6 [object Float64Array] 58595a5b5c5d5e5f
17 5 7 [object Float64Array] 58595a5b5c5d5e5f
17 6 0 [object Float64Array] 
17 6 1 [object Float64Array] 
17 6 2 [object Float64Array] 
17 6 3 [object Float64Array] 
17 6 4 [object Float64Array] 
17 6 5 [object Float64Array] 
17 6 6 [object Float64Array] 
17 6 7 [object Float64Array] 
17 7 0 [object Float64Array] 
17 7 1 [object Float64Array] 
17 7 2 [object Float64Array] 
17 7 3 [object Float64Array] 
17 7 4 [object Float64Array] 
17 7 5 [object Float64Array] 
17 7 6 [object Float64Array] 
17 7 7 [object Float64Array] 
===*/

function subarrayBruteforceTest() {
    var buf = new ArrayBuffer(32);
    var tmp;
    var i;

    tmp = new Uint8Array(buf);
    for (i = 0; i < tmp.length; i++) {
        tmp[i] = 0x40 + i;
    }

    // Include both 1:1 mapped views and offsetted views, we want to test
    // both with subarray().

    var views = [
        new Int8Array(buf),
        new Int8Array(buf, 5, 13),

        new Uint8Array(buf),
        new Uint8Array(buf, 5, 13),

        new Uint8ClampedArray(buf),
        new Uint8ClampedArray(buf, 5, 13),

        new Int16Array(buf),
        new Int16Array(buf, 4, 8),

        new Uint16Array(buf),
        new Uint16Array(buf, 12, 8),

        new Int32Array(buf),
        new Int32Array(buf, 4, 4),

        new Uint32Array(buf),
        new Uint32Array(buf, 12, 4),

        new Float32Array(buf),
        new Float32Array(buf, 4, 4),

        new Float64Array(buf),
        new Float64Array(buf, 8, 3),
    ];

    var offsets = [
        'NONE', -3, -1, 0, 1, 2, '3', 10
    ];

    views.forEach(function (view, idx1) {
        offsets.forEach(function (start, idx2) {
            offsets.forEach(function (end, idx3) {
                var sub;
                try {
                    if (start === 'NONE') {
                        sub = view.subarray();
                    } else if (end === 'NONE') {
                        sub = view.subarray(start);
                    } else {
                        sub = view.subarray(start, end);
                    }
                    print(idx1, idx2, idx3, Object.prototype.toString.call(sub), printableTypedArray(sub));
                } catch (e) {
                    print(idx1, idx2, idx3, Object.prototype.toString.call(sub), e.name, printableTypedArray(sub));
                }
            });
        });
    });
}

try {
    print('bruteforce test');
    subarrayBruteforceTest();
} catch (e) {
    print(e.stack || e);
}
