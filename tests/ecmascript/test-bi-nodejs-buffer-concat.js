/*
 *  Buffer.concat()
 */

/*@include util-nodejs-buffer.js@*/

/*===
concat test
array length: 0, totalLength: undefined
true object
0 bytes: 
0 bytes: 
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 0, totalLength: 0
true object
0 bytes: 
0 bytes: 
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 0, totalLength: 10
true object
0 bytes: 
0 bytes: 
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 1, totalLength: undefined
true object
1 bytes: 22
1 bytes: 5a
0 bytes: 
1 bytes: 5a
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 1, totalLength: 0
true object
1 bytes: 22
1 bytes: 5a
0 bytes: 
1 bytes: 5a
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 1, totalLength: 10
true object
1 bytes: 22
1 bytes: 5a
0 bytes: 
1 bytes: 5a
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: undefined
true object
25 bytes: 224142434445464748494a4b4c4d4e4f504445464748494a4b
25 bytes: 5a4142434445464748494a4b4c4d4e4f504445464748494a4b
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: -1
RangeError
array length: 4, totalLength: null
true object
0 bytes: 
0 bytes: 
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: false
true object
0 bytes: 
0 bytes: 
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: true
true object
1 bytes: 22
1 bytes: 5a
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: dummy totalLength
true object
0 bytes: 
0 bytes: 
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: 17
true object
17 bytes: 224142434445464748494a4b4c4d4e4f50
17 bytes: 5a4142434445464748494a4b4c4d4e4f50
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: 0
true object
0 bytes: 
0 bytes: 
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: 1
true object
1 bytes: 22
1 bytes: 5a
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: 5
true object
5 bytes: 2241424344
5 bytes: 5a41424344
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: 24
true object
24 bytes: 224142434445464748494a4b4c4d4e4f504445464748494a
24 bytes: 5a4142434445464748494a4b4c4d4e4f504445464748494a
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: 25
true object
25 bytes: 224142434445464748494a4b4c4d4e4f504445464748494a4b
25 bytes: 5a4142434445464748494a4b4c4d4e4f504445464748494a4b
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: 26
true object
26 bytes: 224142434445464748494a4b4c4d4e4f504445464748494a4b00
26 bytes: 5a4142434445464748494a4b4c4d4e4f504445464748494a4b00
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 4, totalLength: 100
true object
100 bytes: 224142434445464748494a4b4c4d4e4f504445464748494a4b00000000000000...
100 bytes: 5a4142434445464748494a4b4c4d4e4f504445464748494a4b00000000000000...
0 bytes: 
1 bytes: 22
16 bytes: 4142434445464748494a4b4c4d4e4f50
8 bytes: 4445464748494a4b
array length: 1, totalLength: 1
TypeError
array length: 1, totalLength: 3
TypeError
array length: 2, totalLength: 5
TypeError
array length: 2, totalLength: 5
TypeError
-1 RangeError
0 0 
1 1 f
2 2 fo
3 3 foo
4 4 foob
5 5 fooba
6 6 foobar
7 7 foobarq
8 8 foobarqu
9 9 foobarquu
10 10 foobarquux
11 11 foobarquuxb
12 12 foobarquuxba
13 13 foobarquuxbaz
14 14 foobarquuxbazf
15 15 foobarquuxbazfo
16 16 foobarquuxbazfoo
17 17 foobarquuxbazfoob
18 18 foobarquuxbazfooba
19 19 foobarquuxbazfoobar
20 20 foobarquuxbazfoobarq
21 21 foobarquuxbazfoobarqu
22 22 foobarquuxbazfoobarquu
23 23 foobarquuxbazfoobarquux
24 24 foobarquuxbazfoobarquuxb
25 25 foobarquuxbazfoobarquuxba
26 26 foobarquuxbazfoobarquuxbaz
27 27 foobarquuxbazfoobarquuxbazf
28 28 foobarquuxbazfoobarquuxbazfo
29 29 foobarquuxbazfoobarquuxbazfoo
TypeError
TypeError
TypeError
TypeError
TypeError
===*/

/* Buffer.concat().
 *
 * concat() has an interesting behavior: for an input list of length 1 the
 * first entry is returned (without copying!).  For an input list of length
 * > 1 a new buffer is created.
 */

function concatTest() {
    var b1 = new Buffer(0);
    var b2 = new Buffer(1);
    var b3 = new Buffer(16);
    var b4 = b3.slice(3, 11);  // Test slice in concat too
    var i;

    function test(arr, totalLength) {
        var b, i;
        print('array length: ' + arr.length + ', totalLength: ' + totalLength);

        b1.fill(0x11);
        b2.fill(0x22);
        for (i = 0; i < b3.length; i++) {
            b3[i] = 0x41 + i;
        }
        // b4 is a slice of b3 so no fill

        try {
            if (totalLength === undefined) {
                b = Buffer.concat(arr);
            } else {
                b = Buffer.concat(arr, totalLength);
            }

            print(Buffer.isBuffer(b), typeof b);
            if (Buffer.isBuffer(b)) {
                printNodejsBuffer(b);

                // Write to result and check if it affects input buffers
                // to see if a copy was made.
                b[0] = 0x5a;
                printNodejsBuffer(b);
                printNodejsBuffer(b1);
                printNodejsBuffer(b2);
                printNodejsBuffer(b3);
                printNodejsBuffer(b4);
            }
        } catch (e) {
            //print(e.stack || e);
            print(e.name);
        }
    }

    // Zero length input list; totalLength is ignored and result
    // is a new zero-size buffer.

    test([], undefined);
    test([], 0);
    test([], 10);

    // Length 1; totalLength is ignored and the (only) array
    // element is returned without creating a copy.

    test([b2], undefined);
    test([b2], 0);
    test([b2], 10);

    // Length > 1: totalLength is respected, and a copy is always made.
    // If totalLength is larger than the input buffers combined, the extra
    // data is not initialized and may be non-zero (in Node.js).  In Duktape
    // the extra data is always zeroed.
    //
    // The totalLength value seems to be ToNumber() / ToInteger() coerced
    // by Node.js, because false coerces to 0 and true to 1.

    test([b1, b2, b3, b4], undefined);
    test([b1, b2, b3, b4], -1);
    test([b1, b2, b3, b4], null);
    test([b1, b2, b3, b4], false);
    test([b1, b2, b3, b4], true);
    test([b1, b2, b3, b4], 'dummy totalLength');
    test([b1, b2, b3, b4], '17');
    test([b1, b2, b3, b4], 0);
    test([b1, b2, b3, b4], 1);
    test([b1, b2, b3, b4], 5);
    test([b1, b2, b3, b4], 24);
    test([b1, b2, b3, b4], 25);
    test([b1, b2, b3, b4], 26);  // exceeds input length
    test([b1, b2, b3, b4], 100); // exceeds input length

    // Non-buffer arguments
    //
    // For a single argument Node.js will return the value as is, even
    // if it's not a buffer.  For >= 2 arguments Node.js throws a TypeError
    // but the error messages are odd.

    test(['foo'], 1);  // Node.js returns -string- 'foo'
    test([123], 3);    // Node.js returns -integer- 123
    test(['foo', 'bar'], 5);
    test([123, 234], 5);

    // totalLength boundary condition test; don't exceed input buffer length
    // here because the trailing bytes will be uninitialized (Node.js) or
    // zero (Duktape)
    [ -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
      17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29 ].forEach(function (totalLength) {
        var b1 = new Buffer('foo');
        var b2 = new Buffer('bar');
        var b3 = new Buffer('quux');
        var b4 = new Buffer('baz');
        var buffers = [ b1, b2, b3, b4, b1, b2, b3, b4, b1 ];  // 29 bytes
        var b;
        try {
            b = Buffer.concat(buffers, totalLength);
            print(totalLength, b.length, String(b));
        } catch (e) {
            print(totalLength, e.name);
        }
    });

    // Odd concat arguments; TypeError if first arg not an array
    // (array-like value is also rejected).
    [
        undefined,
        null,
        false,
        true,
        { '0': b1, '1': b2, '2': b3, '3': b4, 'length': 3 }
    ].forEach(function (listarg) {
        var b;
        try {
            b = Buffer.concat(listarg);
            printNodejsBuffer(b);
        } catch (e) {
            print(e.name);
        }
    });
}

try {
    print('concat test');
    concatTest();
} catch (e) {
    print(e.stack || e);
}
