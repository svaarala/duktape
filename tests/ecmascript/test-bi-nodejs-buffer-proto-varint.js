/*
 *  Read/write variable length integer
 */

/*@include util-nodejs-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/* Custom because of differences to Node.js. */

/*===
variable size int test
16 bytes: 00000000000000000000000000000000
16 bytes: 41414141414141414141414141414141
AAAAAAAAAAAAAAAA
[object Buffer]
16
16 bytes: 55555555555555555555555555555555
16 bytes: 5555555555555555aaaa555555555555
4 111111fe111111111111111111111111
5 111111feca1111111111111111111111
6 111111fecaef11111111111111111111
7 111111fecaefbe111111111111111111
8 111111fecaefbead1111111111111111
9 111111fecaefbeadde11111111111111
4 111111fe111111111111111111111111
5 111111feca1111111111111111111111
6 111111fecaef11111111111111111111
7 111111fecaefbe111111111111111111
8 111111fecaefbe991111111111111111
9 111111fecaefbe990011111111111111
4 111111fe111111111111111111111111
5 111111cafe1111111111111111111111
6 111111efcafe11111111111111111111
7 111111beefcafe111111111111111111
8 111111adbeefcafe1111111111111111
9 111111deadbeefcafe11111111111111
4 111111fe111111111111111111111111
5 111111cafe1111111111111111111111
6 111111efcafe11111111111111111111
7 111111beefcafe111111111111111111
8 11111199beefcafe1111111111111111
9 1111110099beefcafe11111111111111
4 111111ab111111111111111111111111
5 111111abbb1111111111111111111111
6 111111abbbcc11111111111111111111
7 111111abbbccdd111111111111111111
8 111111abbbccddee1111111111111111
9 111111abbbccddeeff11111111111111
4 111111ab111111111111111111111111
5 111111bbab1111111111111111111111
6 111111ccbbab11111111111111111111
7 111111ddccbbab111111111111111111
8 111111eeddccbbab1111111111111111
9 111111ffeeddccbbab11111111111111
16 bytes: 111111ff11223344fe11111111111111
1 ff
2 11ff
3 2211ff
4 332211ff
5 44332211ff
6 fe44332211ff
1 ff
2 ff11
3 ff1122
4 ff112233
5 ff11223344
6 ff11223344fe
1 -1
2 11ff
3 2211ff
4 332211ff
5 44332211ff
6 -1bbccddee01
1 -1
2 -ef
3 -eede
4 -eeddcd
5 -eeddccbc
6 -eeddccbb02
noAssert: true, offset: 0, bytelen: 0
0 11111111111111111111111111111111
0 11111111111111111111111111111111
0 11111111111111111111111111111111
0 11111111111111111111111111111111
noAssert: true, offset: 0, bytelen: 6
6 efbeaddefeca11111111111111111111
6 cafedeadbeef11111111111111111111
6 11415221018511111111111111111111
6 85012152411111111111111111111111
noAssert: true, offset: 0, bytelen: 7
7 11111111111111111111111111111111
7 11111111111111111111111111111111
7 11111111111111111111111111111111
7 11111111111111111111111111111111
noAssert: true, offset: 0, bytelen: 10
10 11111111111111111111111111111111
10 11111111111111111111111111111111
10 11111111111111111111111111111111
10 11111111111111111111111111111111
noAssert: true, offset: 10, bytelen: 0
10 11111111111111111111111111111111
10 11111111111111111111111111111111
10 11111111111111111111111111111111
10 11111111111111111111111111111111
noAssert: true, offset: 10, bytelen: 6
16 11111111111111111111efbeaddefeca
16 11111111111111111111cafedeadbeef
16 11111111111111111111114152210185
16 11111111111111111111850121524111
noAssert: true, offset: 10, bytelen: 7
17 11111111111111111111111111111111
17 11111111111111111111111111111111
17 11111111111111111111111111111111
17 11111111111111111111111111111111
noAssert: true, offset: 10, bytelen: 10
20 11111111111111111111111111111111
20 11111111111111111111111111111111
20 11111111111111111111111111111111
20 11111111111111111111111111111111
noAssert: true, offset: 11, bytelen: 0
11 11111111111111111111111111111111
11 11111111111111111111111111111111
11 11111111111111111111111111111111
11 11111111111111111111111111111111
noAssert: true, offset: 11, bytelen: 6
17 11111111111111111111111111111111
17 11111111111111111111111111111111
17 11111111111111111111111111111111
17 11111111111111111111111111111111
noAssert: true, offset: 11, bytelen: 7
18 11111111111111111111111111111111
18 11111111111111111111111111111111
18 11111111111111111111111111111111
18 11111111111111111111111111111111
noAssert: true, offset: 11, bytelen: 10
21 11111111111111111111111111111111
21 11111111111111111111111111111111
21 11111111111111111111111111111111
21 11111111111111111111111111111111
noAssert: true, offset: 16, bytelen: 0
16 11111111111111111111111111111111
16 11111111111111111111111111111111
16 11111111111111111111111111111111
16 11111111111111111111111111111111
noAssert: true, offset: 16, bytelen: 6
22 11111111111111111111111111111111
22 11111111111111111111111111111111
22 11111111111111111111111111111111
22 11111111111111111111111111111111
noAssert: true, offset: 16, bytelen: 7
23 11111111111111111111111111111111
23 11111111111111111111111111111111
23 11111111111111111111111111111111
23 11111111111111111111111111111111
noAssert: true, offset: 16, bytelen: 10
26 11111111111111111111111111111111
26 11111111111111111111111111111111
26 11111111111111111111111111111111
26 11111111111111111111111111111111
noAssert: true, offset: 100, bytelen: 0
100 11111111111111111111111111111111
100 11111111111111111111111111111111
100 11111111111111111111111111111111
100 11111111111111111111111111111111
noAssert: true, offset: 100, bytelen: 6
106 11111111111111111111111111111111
106 11111111111111111111111111111111
106 11111111111111111111111111111111
106 11111111111111111111111111111111
noAssert: true, offset: 100, bytelen: 7
107 11111111111111111111111111111111
107 11111111111111111111111111111111
107 11111111111111111111111111111111
107 11111111111111111111111111111111
noAssert: true, offset: 100, bytelen: 10
110 11111111111111111111111111111111
110 11111111111111111111111111111111
110 11111111111111111111111111111111
110 11111111111111111111111111111111
noAssert: false, offset: 0, bytelen: 0
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 0, bytelen: 6
6 efbeaddefeca11111111111111111111
6 cafedeadbeef11111111111111111111
6 11415221018511111111111111111111
6 85012152411111111111111111111111
noAssert: false, offset: 0, bytelen: 7
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 0, bytelen: 10
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 10, bytelen: 0
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 10, bytelen: 6
16 11111111111111111111efbeaddefeca
16 11111111111111111111cafedeadbeef
16 11111111111111111111114152210185
16 11111111111111111111850121524111
noAssert: false, offset: 10, bytelen: 7
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 10, bytelen: 10
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 11, bytelen: 0
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 11, bytelen: 6
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 11, bytelen: 7
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 11, bytelen: 10
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 16, bytelen: 0
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 16, bytelen: 6
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 16, bytelen: 7
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 16, bytelen: 10
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 100, bytelen: 0
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 100, bytelen: 6
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 100, bytelen: 7
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
noAssert: false, offset: 100, bytelen: 10
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
RangeError 11111111111111111111111111111111
16 bytes: 101112131415161718191a1b1c1d1e1f
noAssert: true, offset: -100, bytelen: 0
NaN
NaN
NaN
NaN
noAssert: true, offset: -100, bytelen: 6
NaN
NaN
NaN
NaN
noAssert: true, offset: -100, bytelen: 7
NaN
NaN
NaN
NaN
noAssert: true, offset: -100, bytelen: 10
NaN
NaN
NaN
NaN
noAssert: true, offset: -6, bytelen: 0
NaN
NaN
NaN
NaN
noAssert: true, offset: -6, bytelen: 6
NaN
NaN
NaN
NaN
noAssert: true, offset: -6, bytelen: 7
NaN
NaN
NaN
NaN
noAssert: true, offset: -6, bytelen: 10
NaN
NaN
NaN
NaN
noAssert: true, offset: -1, bytelen: 0
NaN
NaN
NaN
NaN
noAssert: true, offset: -1, bytelen: 6
NaN
NaN
NaN
NaN
noAssert: true, offset: -1, bytelen: 7
NaN
NaN
NaN
NaN
noAssert: true, offset: -1, bytelen: 10
NaN
NaN
NaN
NaN
noAssert: true, offset: 0, bytelen: 0
NaN
NaN
NaN
NaN
noAssert: true, offset: 0, bytelen: 6
151413121110
101112131415
151413121110
101112131415
noAssert: true, offset: 0, bytelen: 7
NaN
NaN
NaN
NaN
noAssert: true, offset: 0, bytelen: 10
NaN
NaN
NaN
NaN
noAssert: true, offset: 10, bytelen: 0
NaN
NaN
NaN
NaN
noAssert: true, offset: 10, bytelen: 6
1f1e1d1c1b1a
1a1b1c1d1e1f
1f1e1d1c1b1a
1a1b1c1d1e1f
noAssert: true, offset: 10, bytelen: 7
NaN
NaN
NaN
NaN
noAssert: true, offset: 10, bytelen: 10
NaN
NaN
NaN
NaN
noAssert: true, offset: 11, bytelen: 0
NaN
NaN
NaN
NaN
noAssert: true, offset: 11, bytelen: 6
NaN
NaN
NaN
NaN
noAssert: true, offset: 11, bytelen: 7
NaN
NaN
NaN
NaN
noAssert: true, offset: 11, bytelen: 10
NaN
NaN
NaN
NaN
noAssert: true, offset: 16, bytelen: 0
NaN
NaN
NaN
NaN
noAssert: true, offset: 16, bytelen: 6
NaN
NaN
NaN
NaN
noAssert: true, offset: 16, bytelen: 7
NaN
NaN
NaN
NaN
noAssert: true, offset: 16, bytelen: 10
NaN
NaN
NaN
NaN
noAssert: true, offset: 100, bytelen: 0
NaN
NaN
NaN
NaN
noAssert: true, offset: 100, bytelen: 6
NaN
NaN
NaN
NaN
noAssert: true, offset: 100, bytelen: 7
NaN
NaN
NaN
NaN
noAssert: true, offset: 100, bytelen: 10
NaN
NaN
NaN
NaN
noAssert: false, offset: -100, bytelen: 0
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: -100, bytelen: 6
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: -100, bytelen: 7
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: -100, bytelen: 10
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: -6, bytelen: 0
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: -6, bytelen: 6
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: -6, bytelen: 7
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: -6, bytelen: 10
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: -1, bytelen: 0
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: -1, bytelen: 6
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: -1, bytelen: 7
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: -1, bytelen: 10
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 0, bytelen: 0
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 0, bytelen: 6
151413121110
101112131415
151413121110
101112131415
noAssert: false, offset: 0, bytelen: 7
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 0, bytelen: 10
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 10, bytelen: 0
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 10, bytelen: 6
1f1e1d1c1b1a
1a1b1c1d1e1f
1f1e1d1c1b1a
1a1b1c1d1e1f
noAssert: false, offset: 10, bytelen: 7
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 10, bytelen: 10
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 11, bytelen: 0
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 11, bytelen: 6
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 11, bytelen: 7
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 11, bytelen: 10
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 16, bytelen: 0
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 16, bytelen: 6
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 16, bytelen: 7
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 16, bytelen: 10
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 100, bytelen: 0
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 100, bytelen: 6
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 100, bytelen: 7
RangeError
RangeError
RangeError
RangeError
noAssert: false, offset: 100, bytelen: 10
RangeError
RangeError
RangeError
RangeError
===*/

/* Variable size integer read/write calls.
 * Unaligned writes are used, as that is accepted by Node.js Buffer.
 */

function variableSizeIntTest() {
    var b = new Buffer(16);
    var ret;

    // Zeroing is not guaranteed by Node.js, but Duktape guarantees that
    // because the underlying buffer is zeroed by default.
    printNodejsBuffer(b);

    b.fill(0x41);
    printNodejsBuffer(b);
    print(String(b));
    print(Object.prototype.toString.call(b));
    print(b.length);

    b.fill(0x55);
    printNodejsBuffer(b);
    b.fill(0xaa, 8, 10);
    printNodejsBuffer(b);

    // Variable size (u)int write

    for (i = 1; i <= 6; i++) {
        // Use a positive value and check that is extends with zero
        b.fill(0x11);
        ret = b.writeUIntLE(0xdeadbeefcafe, 3, i, true);
        print(ret, printableNodejsBuffer(b));
    }
    for (i = 1; i <= 6; i++) {
        // Use a positive value and check that is extends with zero
        b.fill(0x11);
        ret = b.writeUIntLE(0x99beefcafe, 3, i, true);
        print(ret, printableNodejsBuffer(b));
    }
    for (i = 1; i <= 6; i++) {
        b.fill(0x11);
        ret = b.writeUIntBE(0xdeadbeefcafe, 3, i, true);
        print(ret, printableNodejsBuffer(b));
    }
    for (i = 1; i <= 6; i++) {
        b.fill(0x11);
        ret = b.writeUIntBE(0x99beefcafe, 3, i, true);
        print(ret, printableNodejsBuffer(b));
    }
    for (i = 1; i <= 6; i++) {
        // Use a negative value and check that it sign extends
        b.fill(0x11);
        ret = b.writeIntLE(-0x1122334455, 3, i, true);
        print(ret, printableNodejsBuffer(b));
    }
    for (i = 1; i <= 6; i++) {
        b.fill(0x11);
        ret = b.writeIntBE(-0x1122334455, 3, i, true);
        print(ret, printableNodejsBuffer(b));
    }

    // Variable size (u)int read

    b.fill(0x11);
    b[3] = 0xff;
    b[4] = 0x11;
    b[5] = 0x22;
    b[6] = 0x33;
    b[7] = 0x44;
    b[8] = 0xfe;
    printNodejsBuffer(b);
    for (i = 1; i <= 6; i++) {
        print(i, b.readUIntLE(3, i, true).toString(16));
    }
    for (i = 1; i <= 6; i++) {
        print(i, b.readUIntBE(3, i, true).toString(16));
    }
    for (i = 1; i <= 6; i++) {
        print(i, b.readIntLE(3, i, true).toString(16));
    }
    for (i = 1; i <= 6; i++) {
        print(i, b.readIntBE(3, i, true).toString(16));
    }

    // Writing partially or fully out-of-bounds, valid/invalid length
    //
    // Node.js: when noAssert is false, a negative index causes a RangeError
    // and no change to buffer (not even partial write).  But when the index
    // is positive, within the buffer, but the write extends beyond the end
    // of the buffer, a partial write is done.  (Because of problems with
    // negative indices they've been left out of this test.)
    //
    // When noAssert is true, a RangeError is thrown in both cases, with no
    // change to the buffer.
    //
    // Node.js behavior for bytelen doesn't seem to match the documentation.
    // For bytelen zero, the behavior between LE/BE differs.  For bytelen >= 6
    // writes are allowed and zeroes are written as expected.  Duktape checks
    // are stricter so expect string is based on that.

    [ true, false ].forEach(function (noAssert) {
        [ 0, 10, 11, 16, 100 ].forEach(function (offset) {
            [ 0, 6, 7, 10 ].forEach(function (bytelen) {
                print('noAssert: ' + noAssert + ', offset: ' + offset + ', bytelen: ' + bytelen);

                try {
                    b.fill(0x11);
                    ret = b.writeUIntLE(0xcafedeadbeef, offset, bytelen, noAssert);
                    print(ret, printableNodejsBuffer(b));
                } catch (e) {
                    print(e.name, printableNodejsBuffer(b));
                }

                try {
                    b.fill(0x11);
                    ret = b.writeUIntBE(0xcafedeadbeef, offset, bytelen, noAssert);
                    print(ret, printableNodejsBuffer(b));
                } catch (e) {
                    print(e.name, printableNodejsBuffer(b));
                }

                try {
                    b.fill(0x11);
                    ret = b.writeIntLE(-0x7afedeadbeef, offset, bytelen, noAssert);
                    print(ret, printableNodejsBuffer(b));
                } catch (e) {
                    print(e.name, printableNodejsBuffer(b));
                }

                try {
                    b.fill(0x11);
                    ret = b.writeIntBE(-0x7afedeadbeef, offset, bytelen, noAssert);
                    print(ret, printableNodejsBuffer(b));
                } catch (e) {
                    print(e.name, printableNodejsBuffer(b));
                }
            });
        });
    });

    // Reading partially or fully out-of-bounds, invalid/valid length.
    //
    // Node.js seems to freeze for bytelen -1.  For bytelen 0 the results are
    // weird: one byte is read with LE variant while a TypeError is thrown with
    // the BE variant (!).  Duktape validation is stricter and test case expect
    // string reflects that.

    for (i = 0; i < b.length; i++) {
        b[i] = 0x10 + i;
    }
    printNodejsBuffer(b);

    [ true, false ].forEach(function (noAssert) {
        [ -100, -6, -1, 0, 10, 11, 16, 100 ].forEach(function (offset) {
            [ 0, 6, 7, 10 ].forEach(function (bytelen) {
                print('noAssert: ' + noAssert + ', offset: ' + offset + ', bytelen: ' + bytelen);

                try {
                    print(b.readUIntLE(offset, bytelen, noAssert).toString(16));
                } catch (e) {
                    print(e.name);
                }

                try {
                    print(b.readUIntBE(offset, bytelen, noAssert).toString(16));
                } catch (e) {
                    print(e.name);
                }

                try {
                    print(b.readIntLE(offset, bytelen, noAssert).toString(16));
                } catch (e) {
                    print(e.name);
                }

                try {
                    print(b.readIntBE(offset, bytelen, noAssert).toString(16));
                } catch (e) {
                    print(e.name);
                }
            });
        });
    });
}

try {
    print('variable size int test');
    variableSizeIntTest();
} catch (e) {
    print(e.stack || e);
}
