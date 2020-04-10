'use strict';

const { assert } = require('../util/assert');
const { bufferToUint8Array } = require('../util/buffer');

function utf8Uint8ArrayToString(u8) {
    if (typeof TextDecoder === 'function') {
        // Duktape or recent Node.js
        let data = new TextDecoder().decode(u8);
        assert(typeof data === 'string');
        return data;
    } else {
        // Older Node.js
        assert(typeof Buffer === 'function');
        let data = Buffer.from(u8).toString('utf-8');
        assert(typeof data === 'string');
        return data;
    }
}
exports.utf8Uint8ArrayToString = utf8Uint8ArrayToString;

function stringToUtf8Uint8Array(data) {
    if (typeof TextEncoder === 'function') {
        // Duktape or recent Node.js
        let u8 = new TextEncoder().encode(data);
        assert(u8 instanceof Uint8Array);
        return u8;
    } else {
        // Older Node.js
        assert(typeof Buffer === 'function');
        let u8 = bufferToUint8Array(Buffer.from(data, 'utf-8'));
        assert(u8 instanceof Uint8Array);
        return u8;
    }
}
exports.stringToUtf8Uint8Array = stringToUtf8Uint8Array;
