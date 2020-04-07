'use strict';

const { assert } = require('./assert');

var zeroes = '0'.repeat(256);

function hexEncodeHelper(val, minDigits) {
    function handleU8(u8) {
        var tmp = [];
        for (let i = 0; i < u8.length; i++) {
            tmp.push(hexEncodeHelper(u8[i], 2));
        }
        return tmp.join('');
    }

    if (typeof val === 'number') {
        if (typeof minDigits !== 'number' || minDigits < 1 || minDigits > 256) {
            throw new TypeError('invalid argument');
        }
        var t = val.toString(16);
        if (t.length < minDigits) {
            t = (zeroes + t).substr(-minDigits);
        }
        return t;
    } else if (typeof val === 'object' && val !== null && val instanceof ArrayBuffer) {
        return handleU8(new Uint8Array(val));
    } else if (typeof val === 'object' && val !== null && val instanceof Uint8Array) {
        return handleU8(val);
    } else {
        throw new TypeError('invalid argument');
    }
}

function hexEncodeLower(val, minDigits) {
    return hexEncodeHelper(val, minDigits).toLowerCase();
}
exports.hexEncodeLower = hexEncodeLower;

function hexEncodeUpper(val, minDigits) {
    return hexEncodeHelper(val, minDigits).toUpperCase();
}
exports.hexEncodeUpper = hexEncodeUpper;

const hexEncode = hexEncodeLower;
exports.hexEncode = hexEncode;

function hexDecode(val) {
    assert(typeof val === 'string');
    assert((val.length % 2) === 0);
    return new Uint8Array(val.match(/../g).map((c) => Number.parseInt(c, 16)));
}
exports.hexDecode = hexDecode;
