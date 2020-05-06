'use strict';

const { assert } = require('./assert');
const { hexDecode, hexEncodeLower } = require('./hex');

// Split an IEEE double given as a big endian Uint8Array into logical components.
// Mantissa can be represented accurately as a double.
function splitU8Double(u8) {
    assert(u8 instanceof Uint8Array);
    assert(u8.length === 8);
    var sign = (u8[0] & 0x80) >>> 7;
    var exponent = ((u8[0] & 0x7f) << 4) + ((u8[1] & 0xf0) >>> 4);
    var mantissa = (u8[1] & 0x0f) >>> 0;
    for (let i = 2; i < 8; i++) {
        mantissa = (mantissa * 256) + u8[i];
    }
    return { sign, exponent, mantissa };
}
exports.splitU8Double = splitU8Double;

function splitHexDouble(hexbytes) {
    assert(typeof hexbytes === 'string');
    var u8 = hexDecode(hexbytes);
    if (u8.length !== 8) {
        throw new TypeError('invalid bytes for double value: ' + hexbytes);
    }
    return splitU8Double(u8);
}
exports.splitHexDouble = splitHexDouble;

function splitDouble(v) {
    assert(typeof v === 'number');
    var u8 = new Uint8Array(8);
    new DataView(u8.buffer).setFloat64(0, v);
    return splitU8Double(u8);
}
exports.splitDouble = splitDouble;

function numberToDoubleHex(v) {
    var ab = new ArrayBuffer(8);
    var dv = new DataView(ab);
    dv.setFloat64(0, v);
    return hexEncodeLower(new Uint8Array(ab));
}
exports.numberToDoubleHex = numberToDoubleHex;

function isWhole(x) {
    return typeof x === 'number' && !Number.isNaN(x) && Math.floor(x) === x;
}
exports.isWhole = isWhole;

function isWholeFinite(x) {
    return isWhole(x) && x !== Number.POSITIVE_INFINITY && x !== Number.NEGATIVE_INFINITY;
}
exports.isWholeFinite = isWholeFinite;

function isPowerOfTwo(x) {
    // For normals mantissa is zero because of an implicit leading 1-bit.
    // Denormals are not whole numbers.
    return isWholeFinite(x) && x > 0 && splitDouble(x).mantissa === 0;
}
exports.isPowerOfTwo = isPowerOfTwo;

// Shuffle bytes of a big endian double (Uint8Array) into indicated
// byte order.
function shuffleDoubleBytesFromBigEndian(src, dst, doubleByteOrder) {
    assert(src instanceof Uint8Array);
    assert(dst instanceof Uint8Array);
    assert(src !== dst);
    assert(typeof doubleByteOrder === 'string');

    var indexList = ({
        big:    [ 0, 1, 2, 3, 4, 5, 6, 7 ],
        little: [ 7, 6, 5, 4, 3, 2, 1, 0 ],
        mixed:  [ 3, 2, 1, 0, 7, 6, 5, 4 ]
    })[doubleByteOrder];
    assert(indexList);

    for (let i = 0; i < indexList.length; i++) {
        dst[i] = src[indexList[i]];
    }
}
exports.shuffleDoubleBytesFromBigEndian = shuffleDoubleBytesFromBigEndian;
