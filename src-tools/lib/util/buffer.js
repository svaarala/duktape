'use strict';

const { assert } = require('./assert');

function bufferToArrayBuffer(buf) {
    assert(buf instanceof Buffer);
    return buf.buffer.slice(buf.byteOffset, buf.byteOffset + buf.byteLength);
}
exports.bufferToArrayBuffer = bufferToArrayBuffer;

function bufferToUint8Array(buf) {
    assert(buf instanceof Buffer);
    return new Uint8Array(bufferToArrayBuffer(buf));
}
exports.bufferToUint8Array = bufferToUint8Array;

function ensureUint8ArrayOrNullish(x) {
    if (x instanceof Buffer) {
        return bufferToUint8Array(x);
    } else if (x instanceof Uint8Array) {
        return x;
    }
    assert(x === void 0 || x === null);
    return x;
}
exports.ensureUint8ArrayOrNullish = ensureUint8ArrayOrNullish;
