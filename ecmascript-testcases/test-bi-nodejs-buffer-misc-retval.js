/*
 *  Return values from various write primitives.
 */

/*===
write field
51
51
52
52
52
52
54
54
54
54
54
54
58
58
write varint
51
52
53
54
55
56
51
52
53
54
55
56
51
52
53
54
55
56
51
52
53
54
55
56
write()
6
copy()
23
===*/

function writeReturnValueTest() {
    var buf = new Buffer(256);

    // The basic write field pritimives return "next offset", i.e. input
    // offset + number of bytes written.

    print('write field');
    print(buf.writeUInt8(0x01, 50));
    print(buf.writeInt8(0x01, 50));
    print(buf.writeUInt16BE(0x01, 50));
    print(buf.writeUInt16LE(0x01, 50));
    print(buf.writeInt16BE(0x01, 50));
    print(buf.writeInt16LE(0x01, 50));
    print(buf.writeUInt32BE(0x01, 50));
    print(buf.writeUInt32LE(0x01, 50));
    print(buf.writeInt32BE(0x01, 50));
    print(buf.writeInt32LE(0x01, 50));
    print(buf.writeFloatBE(0x01, 50));
    print(buf.writeFloatLE(0x01, 50));
    print(buf.writeDoubleBE(0x01, 50));
    print(buf.writeDoubleLE(0x01, 50));

    // Variable-sized integer write follows the same principle.

    print('write varint');
    print(buf.writeUIntBE(0x01, 50, 1));
    print(buf.writeUIntBE(0x01, 50, 2));
    print(buf.writeUIntBE(0x01, 50, 3));
    print(buf.writeUIntBE(0x01, 50, 4));
    print(buf.writeUIntBE(0x01, 50, 5));
    print(buf.writeUIntBE(0x01, 50, 6));
    print(buf.writeUIntLE(0x01, 50, 1));
    print(buf.writeUIntLE(0x01, 50, 2));
    print(buf.writeUIntLE(0x01, 50, 3));
    print(buf.writeUIntLE(0x01, 50, 4));
    print(buf.writeUIntLE(0x01, 50, 5));
    print(buf.writeUIntLE(0x01, 50, 6));
    print(buf.writeIntBE(0x01, 50, 1));
    print(buf.writeIntBE(0x01, 50, 2));
    print(buf.writeIntBE(0x01, 50, 3));
    print(buf.writeIntBE(0x01, 50, 4));
    print(buf.writeIntBE(0x01, 50, 5));
    print(buf.writeIntBE(0x01, 50, 6));
    print(buf.writeIntLE(0x01, 50, 1));
    print(buf.writeIntLE(0x01, 50, 2));
    print(buf.writeIntLE(0x01, 50, 3));
    print(buf.writeIntLE(0x01, 50, 4));
    print(buf.writeIntLE(0x01, 50, 5));
    print(buf.writeIntLE(0x01, 50, 6));

    // The write() call is different: it returns number of bytes written,
    // -not- an offset.

    print('write()');
    print(buf.write('foobar', 50));

    // The copy() call also return the number of bytes written (after
    // clipping).

    print('copy()');
    var buf2 = new Buffer(160);
    print(buf2.copy(buf, 233));
}

try {
    writeReturnValueTest();
} catch (e) {
    print(e.stack || e);
}
