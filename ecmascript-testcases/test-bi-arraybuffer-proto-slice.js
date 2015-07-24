/*
 *  ArrayBuffer.prototype.slice()
 */

/*@include util-typedarray.js@*/

/*---
{
    "custom": true
}
---*/

/*===
false 4 41424344
false 3 424344
false 1 44
false 0 
false 4 41424344
false 2 4243
false 2 4243
false 3 424344
false 0 
false 0 
===*/

function arrayBufferSliceBasicTest() {
    var b;

    var buf = new ArrayBuffer(4);
    buf[0] = 0x41; buf[1] = 0x42; buf[2] = 0x43; buf[3] = 0x44;  // ABCD

    // No arguments, return copy.
    b = buf.slice();
    print(b === buf, b.byteLength, printableTypedArray(b));

    // Single argument, offset.
    b = buf.slice(1);
    print(b === buf, b.byteLength, printableTypedArray(b));

    // Negative argument is interpreted from end of buffer.  This is not
    // clearly specified in Khronos specification (it talks about clamping)
    // but ES6 clarifies handling for negative indices:
    // http://www.ecma-international.org/ecma-262/6.0/index.html#sec-%typedarray%.prototype.slice

    b = buf.slice(-1);
    print(b === buf, b.byteLength, printableTypedArray(b));

    // Indices are clamped, after taking account the negative number handling.

    b = buf.slice(10);
    print(b === buf, b.byteLength, printableTypedArray(b));

    b = buf.slice(-100);
    print(b === buf, b.byteLength, printableTypedArray(b));

    // End argument behaves similarly.

    b = buf.slice(1, 3);
    print(b === buf, b.byteLength, printableTypedArray(b));

    b = buf.slice(1, -1);
    print(b === buf, b.byteLength, printableTypedArray(b));

    b = buf.slice(1, 10);
    print(b === buf, b.byteLength, printableTypedArray(b));

    b = buf.slice(1, -100);
    print(b === buf, b.byteLength, printableTypedArray(b));

    // Crossed indices result in a zero size buffer.

    b = buf.slice(3, 2);
    print(b === buf, b.byteLength, printableTypedArray(b));
}

try {
    arrayBufferSliceBasicTest();
} catch (e) {
    print(e.stack || e);
}

/*===
ArrayBuffer slice() bruteforce test
0 0 0 TypeError
0 0 1 TypeError
0 0 2 TypeError
0 0 3 TypeError
0 0 4 TypeError
0 0 5 TypeError
0 1 0 TypeError
0 1 1 TypeError
0 1 2 TypeError
0 1 3 TypeError
0 1 4 TypeError
0 1 5 TypeError
0 2 0 TypeError
0 2 1 TypeError
0 2 2 TypeError
0 2 3 TypeError
0 2 4 TypeError
0 2 5 TypeError
0 3 0 TypeError
0 3 1 TypeError
0 3 2 TypeError
0 3 3 TypeError
0 3 4 TypeError
0 3 5 TypeError
0 4 0 TypeError
0 4 1 TypeError
0 4 2 TypeError
0 4 3 TypeError
0 4 4 TypeError
0 4 5 TypeError
0 5 0 TypeError
0 5 1 TypeError
0 5 2 TypeError
0 5 3 TypeError
0 5 4 TypeError
0 5 5 TypeError
0 6 0 TypeError
0 6 1 TypeError
0 6 2 TypeError
0 6 3 TypeError
0 6 4 TypeError
0 6 5 TypeError
0 7 0 TypeError
0 7 1 TypeError
0 7 2 TypeError
0 7 3 TypeError
0 7 4 TypeError
0 7 5 TypeError
1 0 0 object 0 0 0 1 
1 0 1 object 0 0 0 1 
1 0 2 object 0 0 0 1 
1 0 3 object 0 0 0 1 
1 0 4 object 0 0 0 1 
1 0 5 object 0 0 0 1 
1 1 0 object 0 0 0 1 
1 1 1 object 0 0 0 1 
1 1 2 object 0 0 0 1 
1 1 3 object 0 0 0 1 
1 1 4 object 0 0 0 1 
1 1 5 object 0 0 0 1 
1 2 0 object 0 0 0 1 
1 2 1 object 0 0 0 1 
1 2 2 object 0 0 0 1 
1 2 3 object 0 0 0 1 
1 2 4 object 0 0 0 1 
1 2 5 object 0 0 0 1 
1 3 0 object 0 0 0 1 
1 3 1 object 0 0 0 1 
1 3 2 object 0 0 0 1 
1 3 3 object 0 0 0 1 
1 3 4 object 0 0 0 1 
1 3 5 object 0 0 0 1 
1 4 0 object 0 0 0 1 
1 4 1 object 0 0 0 1 
1 4 2 object 0 0 0 1 
1 4 3 object 0 0 0 1 
1 4 4 object 0 0 0 1 
1 4 5 object 0 0 0 1 
1 5 0 object 0 0 0 1 
1 5 1 object 0 0 0 1 
1 5 2 object 0 0 0 1 
1 5 3 object 0 0 0 1 
1 5 4 object 0 0 0 1 
1 5 5 object 0 0 0 1 
1 6 0 object 0 0 0 1 
1 6 1 object 0 0 0 1 
1 6 2 object 0 0 0 1 
1 6 3 object 0 0 0 1 
1 6 4 object 0 0 0 1 
1 6 5 object 0 0 0 1 
1 7 0 object 0 0 0 1 
1 7 1 object 0 0 0 1 
1 7 2 object 0 0 0 1 
1 7 3 object 0 0 0 1 
1 7 4 object 0 0 0 1 
1 7 5 object 0 0 0 1 
2 0 0 object 4 4 0 1 41424344
2 0 1 object 4 4 0 1 41424344
2 0 2 object 4 4 0 1 41424344
2 0 3 object 4 4 0 1 41424344
2 0 4 object 4 4 0 1 41424344
2 0 5 object 4 4 0 1 41424344
2 1 0 object 4 4 0 1 41424344
2 1 1 object 3 3 0 1 414243
2 1 2 object 0 0 0 1 
2 1 3 object 1 1 0 1 41
2 1 4 object 4 4 0 1 41424344
2 1 5 object 4 4 0 1 41424344
2 2 0 object 1 1 0 1 44
2 2 1 object 0 0 0 1 
2 2 2 object 0 0 0 1 
2 2 3 object 0 0 0 1 
2 2 4 object 1 1 0 1 44
2 2 5 object 1 1 0 1 44
2 3 0 object 4 4 0 1 41424344
2 3 1 object 3 3 0 1 414243
2 3 2 object 0 0 0 1 
2 3 3 object 1 1 0 1 41
2 3 4 object 4 4 0 1 41424344
2 3 5 object 4 4 0 1 41424344
2 4 0 object 3 3 0 1 424344
2 4 1 object 2 2 0 1 4243
2 4 2 object 0 0 0 1 
2 4 3 object 0 0 0 1 
2 4 4 object 3 3 0 1 424344
2 4 5 object 3 3 0 1 424344
2 5 0 object 1 1 0 1 44
2 5 1 object 0 0 0 1 
2 5 2 object 0 0 0 1 
2 5 3 object 0 0 0 1 
2 5 4 object 1 1 0 1 44
2 5 5 object 1 1 0 1 44
2 6 0 object 0 0 0 1 
2 6 1 object 0 0 0 1 
2 6 2 object 0 0 0 1 
2 6 3 object 0 0 0 1 
2 6 4 object 0 0 0 1 
2 6 5 object 0 0 0 1 
2 7 0 object 0 0 0 1 
2 7 1 object 0 0 0 1 
2 7 2 object 0 0 0 1 
2 7 3 object 0 0 0 1 
2 7 4 object 0 0 0 1 
2 7 5 object 0 0 0 1 
3 0 0 object 8 8 0 1 6162636465666768
3 0 1 object 8 8 0 1 6162636465666768
3 0 2 object 8 8 0 1 6162636465666768
3 0 3 object 8 8 0 1 6162636465666768
3 0 4 object 8 8 0 1 6162636465666768
3 0 5 object 8 8 0 1 6162636465666768
3 1 0 object 8 8 0 1 6162636465666768
3 1 1 object 7 7 0 1 61626364656667
3 1 2 object 0 0 0 1 
3 1 3 object 1 1 0 1 61
3 1 4 object 5 5 0 1 6162636465
3 1 5 object 8 8 0 1 6162636465666768
3 2 0 object 1 1 0 1 68
3 2 1 object 0 0 0 1 
3 2 2 object 0 0 0 1 
3 2 3 object 0 0 0 1 
3 2 4 object 0 0 0 1 
3 2 5 object 1 1 0 1 68
3 3 0 object 8 8 0 1 6162636465666768
3 3 1 object 7 7 0 1 61626364656667
3 3 2 object 0 0 0 1 
3 3 3 object 1 1 0 1 61
3 3 4 object 5 5 0 1 6162636465
3 3 5 object 8 8 0 1 6162636465666768
3 4 0 object 7 7 0 1 62636465666768
3 4 1 object 6 6 0 1 626364656667
3 4 2 object 0 0 0 1 
3 4 3 object 0 0 0 1 
3 4 4 object 4 4 0 1 62636465
3 4 5 object 7 7 0 1 62636465666768
3 5 0 object 5 5 0 1 6465666768
3 5 1 object 4 4 0 1 64656667
3 5 2 object 0 0 0 1 
3 5 3 object 0 0 0 1 
3 5 4 object 2 2 0 1 6465
3 5 5 object 5 5 0 1 6465666768
3 6 0 object 1 1 0 1 68
3 6 1 object 0 0 0 1 
3 6 2 object 0 0 0 1 
3 6 3 object 0 0 0 1 
3 6 4 object 0 0 0 1 
3 6 5 object 1 1 0 1 68
3 7 0 object 0 0 0 1 
3 7 1 object 0 0 0 1 
3 7 2 object 0 0 0 1 
3 7 3 object 0 0 0 1 
3 7 4 object 0 0 0 1 
3 7 5 object 0 0 0 1 
===*/

function arrayBufferSliceBruteForceTest() {
    /*
     *  Some differences to Khronos spec:
     *
     *    - First argument defaults to zero if missing (mandatory in spec)
     */

    var i;

    var b0 = new ArrayBuffer(0);
    b0.name = 'b0';

    var b1 = new ArrayBuffer(4);
    b1.name = 'b1';
    for (i = 0; i < 4; i++) { b1[i] = 0x41 + i; }  // ABCD

    var b2 = new ArrayBuffer(8);
    b2.name = 'b2';
    for (i = 0; i < 8; i++) { b2[i] = 0x61 + i; }  // abcdefgh

    [ 123, b0, b1, b2 ].forEach(function (thisValue, idx1) {
        [ 'NONE', -10, -1, 0, 1, 3.9, 7.1, 15 ].forEach(function (begin, idx2) {
            [ 'NONE', -1, 0, 1.9, 5, 9 ].forEach(function (end, idx3) {
                var b;
                try {
                    if (begin === 'NONE') {
                        b = thisValue.slice();
                    } else if (end === 'NONE') {
                        b = thisValue.slice(begin);
                    } else {
                        b = thisValue.slice(begin, end);
                    }
                    print(idx1, idx2, idx3, typeof b, b.length, b.byteLength,
                          b.byteOffset, b.BYTES_PER_ELEMENT,
                          printableTypedArray(b));
                } catch (e) {
                    print(idx1, idx2, idx3, e.name);
                }
            });
        });
    });
}

try {
    print('ArrayBuffer slice() bruteforce test');
    arrayBufferSliceBruteForceTest();
} catch (e) {
    print(e.stack || e);
}
