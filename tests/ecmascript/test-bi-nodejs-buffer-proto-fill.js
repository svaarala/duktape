/*
 *  buf.fill()
 */

/*@include util-nodejs-buffer.js@*/
/*@include util-checksum-string.js@*/

// Hack used to get an expect string using Node.js, adjusted
// for clamping semantics differences.  Uncomment manually if
// needed.
function setHackFill() {
    var orig = Buffer.prototype.fill;
    Buffer.prototype.fill = function fill(value, offset, end) {
        if (typeof offset === 'number' && offset < 0) { offset = 0; }
        if (typeof offset === 'number' && offset >= this.length) { offset = this.length; }
        if (typeof end === 'number' && end < 0) { end = 0; }
        if (typeof end === 'number' && end >= this.length) { end = this.length; }
        return orig.call(this, value, offset, end);
    };
}
//setHackFill();  // uncomment manually

/*===
fill test
basic
16 bytes: 11111122221111111111111111111111
16 bytes: 00000000000000000000000000000000
16 bytes: 11111105050505050505050505050505
16 bytes: 11111111111111111111111111111111
16 bytes: 11111111111111111111111111111111
16 bytes: 11111105111111111111111111111111
16 bytes: 11111111111111111111111111111111
16 bytes: 11111111111111111111111111111111
16 bytes: 11111111111111111111111111111111
16 bytes: 11111111111111111111111111111111
16 bytes: 11111105050505050505050505051111
combinations
0 0 460309
0 1 396400
0 2 396727
0 3 413894
0 4 358825
0 5 353041
0 6 353282
0 7 361285
0 8 361430
0 9 361523
0 10 360364
1 0 391474
1 1 338173
1 2 339396
1 3 349303
1 4 299370
1 5 299760
1 6 300007
1 7 301960
1 8 302133
1 9 302260
1 10 300881
2 0 392784
2 1 339193
2 2 340335
2 3 350767
2 4 300694
2 5 301014
2 6 301214
2 7 302496
2 8 302654
2 9 302781
2 10 301627
3 0 416454
3 1 350603
3 2 351376
3 3 356769
3 4 308574
3 5 302942
3 6 303241
3 7 310926
3 8 311041
3 9 311124
3 10 310039
4 0 367644
4 1 308213
4 2 308259
4 3 314341
4 4 257646
4 5 260874
4 6 260580
4 7 254840
4 8 254896
4 9 255057
4 10 255185
5 0 373132
5 1 306301
5 2 306567
5 3 323647
5 4 268498
5 5 255900
5 6 255822
5 7 266888
5 8 266892
5 9 266987
5 10 267083
6 0 478722
6 1 414995
6 2 406920
6 3 418881
6 4 373454
6 5 366028
6 6 359725
6 7 298566
6 8 295601
6 9 295708
6 10 319823
7 0 365327
7 1 305908
7 2 306132
7 3 312116
7 4 255575
7 5 258587
7 6 258451
7 7 254349
7 8 254447
7 9 254608
7 10 254234
8 0 370590
8 1 303813
8 2 304281
8 3 321577
8 4 265912
8 5 253858
8 6 253910
8 7 265930
8 8 265984
8 9 266079
8 10 265697
9 0 370874
9 1 304329
9 2 304797
9 3 321997
9 4 266508
9 5 254350
9 6 254402
9 7 266526
9 8 266580
9 9 266675
9 10 266293
10 0 357087
10 1 298286
10 2 298815
10 3 306900
10 4 256063
10 5 248569
10 6 248488
10 7 255113
10 8 255160
10 9 255261
10 10 255044
11 0 371562
11 1 304767
11 2 305118
11 3 322467
11 4 266828
11 5 254764
11 6 254729
11 7 266076
11 8 266097
11 9 266192
11 10 266071
12 0 371846
12 1 305283
12 2 305634
12 3 322887
12 4 267424
12 5 255256
12 6 255221
12 7 266672
12 8 266693
12 9 266788
12 10 266667
13 0 358021
13 1 299202
13 2 299684
13 3 307822
13 4 257035
13 5 249533
13 6 249363
13 7 255269
13 8 255293
13 9 255394
13 10 255424
===*/

function fillTest() {
    var b = new Buffer(16);

    print('basic');

    // fill() returns the buffer, so that one can chain
    b.fill(0x11).fill(0x22, 3, 5);
    printNodejsBuffer(b);

    b.fill(0x11);
    b.fill();
    printNodejsBuffer(b);

    b.fill(0x11);
    b.fill(0x05, 3);
    printNodejsBuffer(b);

    b.fill(0x11);
    b.fill(0x05, 3, 1);
    printNodejsBuffer(b);

    b.fill(0x11);
    b.fill(0x05, 3, 3);
    printNodejsBuffer(b);

    b.fill(0x11);
    b.fill(0x05, 3, 4);
    printNodejsBuffer(b);

    // fill size zero, inside range
    b.fill(0x11);
    b.fill(0x05, 0, 0);
    printNodejsBuffer(b);

    // fill size zero, inside range
    b.fill(0x11);
    b.fill(0x05, 16, 16);
    printNodejsBuffer(b);

    // fill size zero, outside range
    b.fill(0x11);
    try {
        b.fill(0x05, -1, -1);
    } catch (e) {
        print(e.name);
    }
    printNodejsBuffer(b);

    // fill size zero, outside range
    b.fill(0x11);
    try {
        b.fill(0x05, 17, 17);
    } catch (e) {
        print(e.name);
    }
    printNodejsBuffer(b);

    b.fill(0x11);
    b.fill(0x05, 3, 14.5);  // truncated to 14
    printNodejsBuffer(b);

    print('combinations');
    [ undefined, null, true, false, 0x61, 0x123, -0x123456,
      { valueOf: function () { return 0x41; } },
      3.5, 3.9, 4, 4.5, 4.9, 5 ].forEach(function (value, idx1) {
        [ undefined, null, true, false, -1, 0, 1, 15, 16, 17,
          { valueOf: function () { return 10; } } ].forEach(function (offset, idx2) {
            var tmp = [];

            [ undefined, null, true, false, -1, 0, 1, 15, 16, 17,
              { valueOf: function () { return 10; } } ].forEach(function (end, idx3) {
                try {
                    b.fill(0x11);
                    b.fill(value, offset, end);
                    tmp.push(value + ' ' + offset + ' ' + end + ' ' + printableNodejsBuffer(b));
                } catch (e) {
                    tmp.push(value + ' ' + offset + ' ' + end + ' ' + e.name + ' ' + printableNodejsBuffer(b));
                }
            });

            //print(tmp.join('\n'));
            print(idx1, idx2, checksumString(tmp.join('\n')));
        });
    });
}

try {
    print('fill test');
    fillTest();
} catch (e) {
    print(e.stack || e);
}
