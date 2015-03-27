/*
 *  Write fixed size field
 */

/*@include util-nodejs-buffer.js@*/
/*@include util-checksum-string.js@*/

/*---
{
    "custom": true,
    "endianness": "little"
}
---*/

/* Custom because of Duktape lenient write coercion: out-of-bounds values are
 * silently coerced like with TypedArray while Node.js Buffer throws a TypeError
 * (although it will silently coerce fractions).
 */

/*===
write field test
0 0 18251757
0 1 18165881
0 2 18204851
0 3 18376395
0 4 18084865
0 5 18125131
0 6 17140209
0 7 17189399
0 8 17329551
0 9 17153621
0 10 17212003
0 11 16835591
0 12 16855503
0 13 16902899
0 14 16834507
0 15 16865149
0 16 16035993
0 17 16033385
0 18 16501733
0 19 16411339
0 20 16416253
0 21 16555383
0 22 16413415
0 23 17128497
0 24 16904205
0 25 16909653
0 26 17157489
0 27 16895545
0 28 18225275
0 29 17899009
0 30 17904223
0 31 18313175
0 32 18288797
0 33 17986225
0 34 19183655
0 35 19306891
0 36 16982625
0 37 17051141
0 38 16820677
0 39 16258797
===*/

/* Write field (fixed size) tests. */

function writeFieldTest() {
    // noAssert coercions tested separately.

    [ false ].forEach(function (noAssert, idx1) {
        [
            -0x123456789, -0xffffffff, -0xdeadbeef, -0x80000001, -0x80000000, -0x7fffffff,
            -0x10000, -0xffff, -0x8001, -0x8000, -0x7fff,
            -0x100, -0xff, -0x81, -0x80, -0x7f, -0, 0,
            0x7f, 0x80, 0x81, 0xff, 0x100,
            0x7fff, 0x8000, 0x8001, 0xffff, 0x10000,
            0x7fffffff, 0x80000000, 0x80000001, 0xdeadbeef, 0xffffffff, 0x123456789,
            1 / 0,   /* inf */
            -1 / 0,  /* -inf */

            // Leave NaN intentionally out because the result is platform and
            // engine specific.

            123.9,   /* fraction */
            'foo',   /* string */
            '100.1', /* number-like string */
            { valueOf: function () { return 65; } }  /* coercable object */
        ].forEach(function (value, idx2) {
            var tmp = [];

            [ -100, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3,
              8, 9, 10, 11, 12, 13, 14, 15, 16, 100 ].forEach(function (offset, idx3) {
                [ 'writeUInt8', 'writeUInt16LE', 'writeUInt16BE', 'writeUInt32LE', 'writeUInt32BE',
                  'writeInt8', 'writeInt16LE', 'writeInt16BE', 'writeInt32LE', 'writeInt32BE',
                  'writeFloatLE', 'writeFloatBE', 'writeDoubleLE', 'writeDoubleBE' ].forEach(function (funcname, idx4) {
                    var b = new Buffer(16);
                    b.fill(0x11);

                    var writeValue = value;

// Simulate lenient coercion to get expect output
/*
if (typeof process !== 'undefined' && typeof value === 'number') {
    if (funcname.indexOf('UInt8') >= 0) { writeValue = value & 0xff; }
    else if (funcname.indexOf('Int8') >= 0) { writeValue = ((value & 0xff) << 24) >> 24; }

    if (funcname.indexOf('UInt16') >= 0) { writeValue = value & 0xffff; }
    else if (funcname.indexOf('Int16') >= 0) { writeValue = ((value & 0xffff) << 16) >> 16; }

    if (funcname.indexOf('UInt32') >= 0) { writeValue = value >>> 0; }
    else if (funcname.indexOf('Int32') >= 0) { writeValue = value >> 0; }
}
*/

                    try {
                        b[funcname](writeValue, offset, noAssert);
                        tmp.push(noAssert + ' ' + offset + ' ' + value + ' ' +
                                 funcname + ' ' + printableNodejsBuffer(b));
                    } catch (e) {
// Hackery to get expect string
/*
if (typeof process !== 'undefined' && String(e) === 'TypeError: value is out of bounds') {
    e = new RangeError('dummy');
}
*/
                        tmp.push(noAssert + ' ' + offset + ' ' + value + ' ' +
                                 funcname + ' ' + e.name + ' ' + printableNodejsBuffer(b));
                    }
                });
            });

            print(idx1, idx2, checksumString(tmp.join('\n')));
        });
    });
}

try {
    print('write field test');
    writeFieldTest();
} catch (e) {
    print(e.stack || e);
}

/*===
writefield noAssert coercion test
0 RangeError
8 bytes: 4142434445464748
1 RangeError
8 bytes: 4142434445464748
2 10
8 bytes: 4142434445464748
3 RangeError
8 bytes: 4142434445464748
4 10
8 bytes: 4142434445464748
5 RangeError
8 bytes: 4142434445464748
6 10
8 bytes: 4142434445464748
7 RangeError
8 bytes: 4142434445464748
===*/

/*
 *  noAssert coercion test.
 *
 *  When noAssert=true and the write fails, Node.js Buffer still returns the
 *  same value as if the write succeeded (offset + nbytes).
 */

function writeFieldNoAssertCoercionTest() {
    [ undefined, null, true, false, 123, 0, 'foo', '' ].forEach(function (noAssert, idx) {
        var buf = new Buffer('ABCDEFGH');
        try {
            print(idx, buf.writeUInt32BE(0x61626364, 6, noAssert));
        } catch (e) {
            print(idx, e.name);
        }
        printNodejsBuffer(buf);
    });
}

try {
    print('writefield noAssert coercion test');
    writeFieldNoAssertCoercionTest();
} catch (e) {
    print(e.stack || e);
}

/*===
writefield retval test
8
AAAAAAAaAAAAAAAAAAAAAAAAAAAAAAAA 4141414141414161414141414141414141414141414141414141414141414141
8
AAAAAAAaAAAAAAAAAAAAAAAAAAAAAAAA 4141414141414161414141414141414141414141414141414141414141414141
9
AAAAAAAabAAAAAAAAAAAAAAAAAAAAAAA 4141414141414161624141414141414141414141414141414141414141414141
9
AAAAAAAbaAAAAAAAAAAAAAAAAAAAAAAA 4141414141414162614141414141414141414141414141414141414141414141
9
AAAAAAAabAAAAAAAAAAAAAAAAAAAAAAA 4141414141414161624141414141414141414141414141414141414141414141
9
AAAAAAAbaAAAAAAAAAAAAAAAAAAAAAAA 4141414141414162614141414141414141414141414141414141414141414141
11
AAAAAAAabcdAAAAAAAAAAAAAAAAAAAAA 4141414141414161626364414141414141414141414141414141414141414141
11
AAAAAAAdcbaAAAAAAAAAAAAAAAAAAAAA 4141414141414164636261414141414141414141414141414141414141414141
11
AAAAAAAabcdAAAAAAAAAAAAAAAAAAAAA 4141414141414161626364414141414141414141414141414141414141414141
11
AAAAAAAdcbaAAAAAAAAAAAAAAAAAAAAA 4141414141414164636261414141414141414141414141414141414141414141
11
4141414141414140490fdb414141414141414141414141414141414141414141
11
41414141414141db0f4940414141414141414141414141414141414141414141
15
41414141414141400921fb54442d184141414141414141414141414141414141
15
41414141414141182d4454fb2109404141414141414141414141414141414141
===*/

/*
 *  Node.js write methods return the "new offset" after the write, i.e.
 *  (offset + nbytes).
 */

function writeFieldRetvalTest() {
    var buf = new Buffer(32);

    buf.fill(0x41);
    print(buf.writeUInt8(0x61, 7));
    print(String(buf), printableNodejsBuffer(buf));

    buf.fill(0x41);
    print(buf.writeInt8(0x61, 7));
    print(String(buf), printableNodejsBuffer(buf));

    buf.fill(0x41);
    print(buf.writeUInt16BE(0x6162, 7));
    print(String(buf), printableNodejsBuffer(buf));

    buf.fill(0x41);
    print(buf.writeUInt16LE(0x6162, 7));
    print(String(buf), printableNodejsBuffer(buf));

    buf.fill(0x41);
    print(buf.writeInt16BE(0x6162, 7));
    print(String(buf), printableNodejsBuffer(buf));

    buf.fill(0x41);
    print(buf.writeInt16LE(0x6162, 7));
    print(String(buf), printableNodejsBuffer(buf));

    buf.fill(0x41);
    print(buf.writeUInt32BE(0x61626364, 7));
    print(String(buf), printableNodejsBuffer(buf));

    buf.fill(0x41);
    print(buf.writeUInt32LE(0x61626364, 7));
    print(String(buf), printableNodejsBuffer(buf));

    buf.fill(0x41);
    print(buf.writeInt32BE(0x61626364, 7));
    print(String(buf), printableNodejsBuffer(buf));

    buf.fill(0x41);
    print(buf.writeInt32LE(0x61626364, 7));
    print(String(buf), printableNodejsBuffer(buf));

    // Avoid printing binary

    buf.fill(0x41);
    print(buf.writeFloatBE(Math.PI, 7));
    print(printableNodejsBuffer(buf));

    buf.fill(0x41);
    print(buf.writeFloatLE(Math.PI, 7));
    print(printableNodejsBuffer(buf));

    buf.fill(0x41);
    print(buf.writeDoubleBE(Math.PI, 7));
    print(printableNodejsBuffer(buf));

    buf.fill(0x41);
    print(buf.writeDoubleLE(Math.PI, 7));
    print(printableNodejsBuffer(buf));
}

try {
    print('writefield retval test');
    writeFieldRetvalTest();
} catch (e) {
    print(e.stack || e);
}
