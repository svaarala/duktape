/*
 *  Node.js Buffer slice()
 */

/*@include util-nodejs-buffer.js@*/

/*===
Node.js Buffer slice() test
8 bytes: 4142434445464748
8 bytes: 4142434445464748
ABaDEFGH ABaDEFGH
ABapqbGH ABapqbGH
8 bytes: 4142434445464748
2 bytes: 4344
ABCaEFGH Ca
ABbaEFGH ba
ABbbEFGH bb
ABbbEFGH bb
ABbbEFGH bb
ABbbEFGH bb
ABbbEFGH bb
ABbbEFGH bb
ABbbEFGH bb
32 bytes: 1111111111111111111111111111111111111111111111111111111111111111
17 bytes: 2222222222222222222222222222222222
9 bytes: 333333333333333333
9 bytes: 623333333333333333
9 bytes: 626233333333333333
9 bytes: 626262333333333333
9 bytes: 626262623333333333
9 bytes: 626262626233333333
9 bytes: 626262626262333333
9 bytes: 626262626262623333
9 bytes: 626262626262626233
9 bytes: 626262626262626262
9 bytes: 626262626262626262
9 bytes: 626262626262626262
9 bytes: 626262626262626262
9 bytes: 626262626262626262
9 bytes: 626262626262626262
9 bytes: 626262626262626262
9 bytes: 626262626262626262
32 bytes: 1111626262707162626262222222222222222222111111111111111111111111
17 bytes: 6262707162626262222222222222222222
9 bytes: 626262707162626262
===*/

function nodejsBufferSliceTest() {
    var b1, b2, b3;
    var i;

    // Very basic case, no offsets; check that both refer to the same
    // underlying buffer.

    b1 = new Buffer('ABCDEFGH');
    printNodejsBuffer(b1);
    b2 = b1.slice();
    printNodejsBuffer(b2);
    b1[2] = 0x61;
    print(String(b1), String(b2));
    b2.writeUInt16BE(0x7071, 3);
    b2[5] = 0x62;
    print(String(b1), String(b2));

    // Basic case with an offset and limit; check that can't overrun
    // the parent buffer.

    b1 = new Buffer('ABCDEFGH');
    printNodejsBuffer(b1);
    b2 = b1.slice(2, 4);
    printNodejsBuffer(b2);
    b1[3] = 0x61;
    print(String(b1), String(b2));
    for (i = 0; i < 8; i++) {
        b2[i] = 0x62;
        print(String(b1), String(b2));
    }

    // Slice of a slice

    b1 = new Buffer(32);
    b1.fill(0x11);
    printNodejsBuffer(b1);
    b2 = b1.slice(3, 20);
    b2.fill(0x22);
    printNodejsBuffer(b2);
    b3 = b1.slice(2, 11);
    b3.fill(0x33);
    printNodejsBuffer(b3);
    for (i = 0; i < 16; i++) {
        b3[i] = 0x62;
        printNodejsBuffer(b3);
    }
    b3.writeUInt16BE(0x7071, 3);
    printNodejsBuffer(b1);
    printNodejsBuffer(b2);
    printNodejsBuffer(b3);
}

try {
    print('Node.js Buffer slice() test');
    nodejsBufferSliceTest();
} catch (e) {
    print(e.stack || e);
}
