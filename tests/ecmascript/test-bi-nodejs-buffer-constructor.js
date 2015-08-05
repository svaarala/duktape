/*
 *  Constructor (Buffer)
 *
 *  Test both 'new Buffer()' and 'Buffer()' call styles.  Unlike with the
 *  TypedArray specification, both call styles are accepted.
 */

/*@include util-nodejs-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
constructor test
size: 0
0 bytes: 
[object Buffer] -> Buffer.prototype -> Object.prototype
0 bytes: 
[object Buffer] -> Buffer.prototype -> Object.prototype
size: 16
16 bytes: 00000000000000000000000000000000
[object Buffer] -> Buffer.prototype -> Object.prototype
16 bytes: 00000000000000000000000000000000
[object Buffer] -> Buffer.prototype -> Object.prototype
size: -1
0 bytes: 
[object Buffer] -> Buffer.prototype -> Object.prototype
0 bytes: 
[object Buffer] -> Buffer.prototype -> Object.prototype
size: -Infinity
0 bytes: 
[object Buffer] -> Buffer.prototype -> Object.prototype
0 bytes: 
[object Buffer] -> Buffer.prototype -> Object.prototype
size: NaN
0 bytes: 
[object Buffer] -> Buffer.prototype -> Object.prototype
0 bytes: 
[object Buffer] -> Buffer.prototype -> Object.prototype
size: 1e+50
RangeError
RangeError
size: 4294967296
RangeError
RangeError
size: 4294967295
RangeError
RangeError
size: 1073741824
1073741824 bytes: 0000000000000000000000000000000000000000000000000000000000000000...
[object Buffer] -> Buffer.prototype -> Object.prototype
1073741824 bytes: 0000000000000000000000000000000000000000000000000000000000000000...
[object Buffer] -> Buffer.prototype -> Object.prototype
size: 1073741823
1073741823 bytes: 0000000000000000000000000000000000000000000000000000000000000000...
[object Buffer] -> Buffer.prototype -> Object.prototype
1073741823 bytes: 0000000000000000000000000000000000000000000000000000000000000000...
[object Buffer] -> Buffer.prototype -> Object.prototype
size: 16777216
16777216 bytes: 0000000000000000000000000000000000000000000000000000000000000000...
[object Buffer] -> Buffer.prototype -> Object.prototype
16777216 bytes: 0000000000000000000000000000000000000000000000000000000000000000...
[object Buffer] -> Buffer.prototype -> Object.prototype
size: 268435456
268435456 bytes: 0000000000000000000000000000000000000000000000000000000000000000...
[object Buffer] -> Buffer.prototype -> Object.prototype
268435456 bytes: 0000000000000000000000000000000000000000000000000000000000000000...
[object Buffer] -> Buffer.prototype -> Object.prototype
size: 123
0 bytes: 
[object Buffer] -> Buffer.prototype -> Object.prototype
0 bytes: 
[object Buffer] -> Buffer.prototype -> Object.prototype
array length: 0
0 bytes: 
0 bytes: 
array length: 3
3 bytes: 010203
3 bytes: 010203
array length: 12
12 bytes: 040404040505050506060606
12 bytes: 040404040505050506060606
array length: 12
12 bytes: 00000000fffffffffefefefe
12 bytes: 00000000fffffffffefefefe
array length: 3
3 bytes: 01fedd
3 bytes: 01fedd
array length: 2
2 bytes: 4142
2 bytes: 4142
array length: 3
3 bytes: 112233
3 bytes: 112233
array length: 6
6 bytes: 110033440000
6 bytes: 110033440000
array length: undefined
0 bytes: 
0 bytes: 
buffer length: 4
4 bytes: 11223344
4 bytes: 11223344
4 bytes: 11223344
4 bytes: 1122ff44
string length: 0, encoding: undefined
0 bytes: 
0 bytes: 
string length: 3, encoding: undefined
3 bytes: 666f6f
3 bytes: 666f6f
string length: 7, encoding: utf8
9 bytes: 666f6fe188b4626172
9 bytes: 666f6fe188b4626172
string length: 7, encoding: dummy
9 bytes: 666f6fe188b4626172
9 bytes: 666f6fe188b4626172
string length: 8, encoding: utf8
12 bytes: 666f6feda080eda3bf626172
12 bytes: 666f6feda080eda3bf626172
size: undefined
TypeError
TypeError
size: null
TypeError
TypeError
size: true
TypeError
TypeError
size: false
TypeError
TypeError
noargument TypeError
noargument TypeError
12 bytes: 000000000000000000000000
12 bytes: 000000000000000000000000
===*/

function constructorTest() {
    var b1, b2;

    /*
     *  Size argument
     *
     *  In Node.js v0.12.1 negative values, infinities, and NaN result in a
     *  zero size buffer (not an error).  Values >= 2^32 are also ignored
     *  and result in a zero size buffer.  Values in range [0x40000000,0xffffffff]
     *  result in a RangeError:
     *
     *      RangeError: Attempt to allocate Buffer larger than maximum size: 0x3fffffff bytes
     *
     *  Duktape doesn't replicate this behavior exactly.
     */

    function newSize(sz) {
        var b;
        print('size: ' + sz);

        try {
            b = new Buffer(sz);

            // Node.js doesn't guarantee zeroing but Duktape does, so
            // printing is OK.
            printNodejsBuffer(b);
            printPrototypeChain(b);
        } catch (e) {
            //print(e.stack || e);
            print(e.name);
        }
        b = null;  // free previous buffer before allocating next, avoids out-of-memory for 1G test

        try {
            b = Buffer(sz);
            printNodejsBuffer(b);
            printPrototypeChain(b);
        } catch (e) {
            //print(e.stack || e);
            print(e.name);
        }
    }

    newSize(0);
    newSize(16);
    newSize(-1);
    newSize(-1 / 0);
    newSize(0 / 0);
    newSize(1e50);         // plain Error when allocating buffer (fragile)

    newSize(0x100000000);  // Node.js: no error, zero size result
    newSize(0xffffffff);   // Node.js: curiously, RangeError
    newSize(0x40000000);
    newSize(0x3fffffff);   // ~1G
    newSize(16 * 1024 * 1024);  // 16MB
    newSize(256 * 1024 * 1024);  // 256MB

    // Unlike TypedArray, Node.js doesn't create a 123-byte buffer here
    // but a 0-byte one.  This is presumable because the argument is
    // interpreted like an 'array-like' constructor call with a missing
    // 'length' so that a 0-byte array results.

    newSize({ valueOf: function () { return 123; } });

    /*
     *  Array argument
     *
     *  Simple clamping of arguments is used.  ToNumber() coercion or similar
     *  is applied because an object with a .valueOf() property gets coerced
     *  through that valueOf.
     */

    function newArray(arr) {
        var b;
        print('array length: ' + arr.length);

        try {
            b = new Buffer(arr);
            printNodejsBuffer(b);
        } catch (e) {
            print(e.stack || e);
        }

        try {
            b = Buffer(arr);
            printNodejsBuffer(b);
        } catch (e) {
            print(e.stack || e);
        }
    }

    newArray([]);
    newArray([ 1, 2, 3 ]);
    newArray([ 4.0, 4.1, 4.5, 4.9, 5.0, 5.1, 5.5, 5.9, 6.0, 6.1, 6.5, 6.9 ]);
    newArray([ -0.0, -0.1, -0.5, -0.9, -1.0, -1.1, -1.5, -1.9, -2.0, -2.1, -2.5, -2.9 ]);
    newArray([ 0x101, 0x1fe, -0x123 ]);
    newArray([ { valueOf: function () { return 0x41; } }, { valueOf: function () { return 0x42; } } ]);

    /*
     *  Array-like arguments are also accepted with the usual rules:
     *  'length' must be present.
     */

    newArray({ '0': 0x11, '1': 0x22, '2' : 0x33, '3': 0x44,
               length: 3  /* '3' gets ignored because it's outside length */ });
    newArray({ '0': 0x11, '2' : 0x33, '3': 0x44,
               length: 6  /* '1' is missing (not defined), zero is used, same for '4' and '5' */ });

    /*
     *  If 'length' is missing, a zero-size buffer results.
     */

    newArray({ '0': 0x11, '2' : 0x33, '3': 0x44 });

    /*
     *  Buffer argument
     *
     *  Independent copy gets created.
     */

    function newBuffer(buf) {
        var b;
        print('buffer length: ' + buf.length);

        try {
            b = new Buffer(buf);
            printNodejsBuffer(b);
        } catch (e) {
            print(e.stack || e);
        }

        try {
            b = Buffer(buf);
            printNodejsBuffer(b);
        } catch (e) {
            print(e.stack || e);
        }

        return b;
    }

    b1 = new Buffer(4);
    b1[0] = 0x11; b1[1] = 0x22; b1[2] = 0x33; b1[3] = 0x44;
    b2 = newBuffer(b1);
    b2[2] = 0xff;  // independent copy
    printNodejsBuffer(b1);
    printNodejsBuffer(b2);

    /*
     *  String argument
     *
     *  Duktape ignores encoding and always just writes the internal
     *  extended CESU-8/UTF-8 data into the buffer.  For standard strings
     *  this is equivalent to UTF-8 encoding.
     *
     *  When using 'utf8' with Node.js, U+D800...U+D8FF result in a U+FFFD
     *  (REPLACEMENT CHARACTER), encoded in UTF-8, to be used in the resulting
     *  buffer.  Duktape will CESU-8 encode such characters.
     */

    function newString(str, encoding) {
        var b;
        print('string length: ' + str.length + ', encoding: ' + encoding);

        try {
            if (encoding !== undefined) {
                b = new Buffer(str, encoding);
            } else {
                b = new Buffer(str);
            }
            printNodejsBuffer(b);
        } catch (e) {
            //print(e.stack || e);
            print(e.name);
        }

        try {
            if (encoding !== undefined) {
                b = Buffer(str, encoding);
            } else {
                b = Buffer(str);
            }
            printNodejsBuffer(b);
        } catch (e) {
            //print(e.stack || e);
            print(e.name);
        }
    }

    newString('');
    newString('foo');
    newString('foo\u1234bar', 'utf8');

    // Node.js throws a TypeError for an unsupported encoding.
    // Duktape ignores the encoding argument and creates a buffer
    // using the CESU-8/extended-UTF-8 bytes of the internal
    // representation.
    newString('foo\u1234bar', 'dummy');

    // Node.js replaces surrogate pair codepoints with U+FFFD, Duktape
    // allows them as-is encoding them with CESU-8.
    newString('foo\ud800\ud8ffbar', 'utf8');

    /*
     *  Odd arguments
     */

    newSize(undefined);
    newSize(null);
    newSize(true);
    newSize(false);

    // no argument -> TypeError
    try {
        b1 = new Buffer();
        printNodejsBuffer(b1);
    } catch (e) {
        print('noargument', e.name);
    }
    try {
        b1 = Buffer();
        printNodejsBuffer(b1);
    } catch (e) {
        print('noargument', e.name);
    }

    // extra arguments -> ignored
    try {
        b1 = new Buffer(12, 'dummy');
        printNodejsBuffer(b1);
    } catch (e) {
        print('noargument', e.name);
    }
    try {
        b1 = Buffer(12, 'dummy');
        printNodejsBuffer(b1);
    } catch (e) {
        print('noargument', e.name);
    }
}

try {
    print('constructor test');
    constructorTest();
} catch (e) {
    print(e.stack || e);
}
