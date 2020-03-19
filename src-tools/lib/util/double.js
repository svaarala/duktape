'use strict';

const { assert } = require('./assert');
const { hexDecode } = require('./hex');

function splitU8Double(u8) {
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

function isWhole(x) {
    return typeof x === 'number' && !Number.isNaN(x) && Math.floor(x) === x;
}
exports.isWhole = isWhole;

function isWholeFinite(x) {
    return isWhole(x) && x !== Number.POSITIVE_INFINITY && x !== Number.NEGATIVE_INFINITY;
}
exports.isWholeFinite = isWholeFinite;
