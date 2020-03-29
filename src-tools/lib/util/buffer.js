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
