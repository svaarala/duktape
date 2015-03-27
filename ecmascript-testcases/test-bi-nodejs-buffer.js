/*
 *  Node.js Buffer API tests
 *
 *  https://nodejs.org/api/buffer.html
 *  https://nodejs.org/docs/v0.12.1/api/buffer.html
 *
 *  Tests are based on NodeJS v0.12.1.
 */

/*@include util-nodejs-buffer.js@*/

/*===
Buffer: function
Buffer.name: Buffer
isExtensible(Buffer): true
Object.prototype.toString(Buffer): [object Function]
Buffer.prototype: object
Buffer.prototype.name: undefined
isExtensible(Buffer.prototype): true
Object.prototype.toString(Buffer.prototype): [object Object]
SlowBuffer: undefined
===*/

/* Just a dummy existence test.  Actual testcases are in separate files. */

function test() {
    try {
        print('Buffer:', typeof this.Buffer);
        print('Buffer.name:', this.Buffer.name);
        print('isExtensible(Buffer):', Object.isExtensible(this.Buffer));
        print('Object.prototype.toString(Buffer):', Object.prototype.toString.call(this.Buffer));
    } catch (e) {
        print(e.stack || e);
    }

    try {
        print('Buffer.prototype:', typeof this.Buffer.prototype);
        print('Buffer.prototype.name:', this.Buffer.prototype.name);
        print('isExtensible(Buffer.prototype):', Object.isExtensible(this.Buffer.prototype));
        print('Object.prototype.toString(Buffer.prototype):', Object.prototype.toString.call(this.Buffer.prototype));
    } catch (e) {
        print(e.stack || e);
    }

    try {
        print('SlowBuffer:', typeof this.SlowBuffer);
    } catch (e) {
        print(e.stack || e);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}

/*
 *  Summary of methods and properties
 */

// new Buffer(size)
// new Buffer(array)
// new Buffer(buffer)
// new Buffer(str, [encoding])
// Buffer(size)
// Buffer(array)
// Buffer(buffer)
// Buffer(str, [encoding])

// Buffer.isEncoding(encoding)
// Buffer.isBuffer(obj)
// Buffer.byteLength(str, [encoding])
// Buffer.concat(list, [totalLength])
// Buffer.compare(buf1, buf2)

// buf.length
// buf.write(string, [offset], [length], [encoding])
// buf.writeUIntLE(value, offset, byteLength, [noAssert])
// buf.writeUIntBE(value, offset, byteLength, [noAssert])
// buf.writeIntLE(value, offset, byteLength, [noAssert])
// buf.writeIntBE(value, offset, byteLength, [noAssert])
// buf.readUIntLE(offset, byteLength, [noAssert])
// buf.readUIntBE(offset, byteLength, [noAssert])
// buf.readIntLE(offset, byteLength, [noAssert])
// buf.readIntBE(offset, byteLength, [noAssert])
// buf.toString([encoding], [start], [end])
// buf.toJSON()
// buf[index]
// buf.equals(otherBuffer)
// buf.compare(otherBuffer)
// buf.copy(targetBuffer, [targetStart], [sourceStart], [sourceEnd])
// buf.slice([start], [end])
// buf.readUInt8(offset, [noAssert])
// buf.readUInt16LE(offset, [noAssert])
// buf.readUInt16BE(offset, [noAssert])
// buf.readUInt32LE(offset, [noAssert])
// buf.readUInt32BE(offset, [noAssert])
// buf.readInt8(offset, [noAssert])
// buf.readInt16LE(offset, [noAssert])
// buf.readInt16BE(offset, [noAssert])
// buf.readInt32LE(offset, [noAssert])
// buf.readInt32BE(offset, [noAssert])
// buf.readFloatLE(offset, [noAssert])
// buf.readFloatBE(offset, [noAssert])
// buf.readDoubleLE(offset, [noAssert])
// buf.readDoubleBE(offset, [noAssert])
// buf.writeUInt8(value, offset, [noAssert])
// buf.writeUInt16LE(value, offset, [noAssert])
// buf.writeUInt16BE(value, offset, [noAssert])
// buf.writeUInt32LE(value, offset, [noAssert])
// buf.writeUInt32BE(value, offset, [noAssert])
// buf.writeInt8(value, offset, [noAssert])
// buf.writeInt16LE(value, offset, [noAssert])
// buf.writeInt16BE(value, offset, [noAssert])
// buf.writeInt32LE(value, offset, [noAssert])
// buf.writeInt32BE(value, offset, [noAssert])
// buf.writeFloatLE(value, offset, [noAssert])
// buf.writeFloatBE(value, offset, [noAssert])
// buf.writeDoubleLE(value, offset, [noAssert])
// buf.writeDoubleBE(value, offset, [noAssert])
// buf.fill(value, [offset], [end])
