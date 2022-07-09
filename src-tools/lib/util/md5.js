'use strict';

const { assert } = require('./assert');
const { bufferToArrayBuffer } = require('./buffer');

function md5(data) {
    assert(data instanceof ArrayBuffer);
    let buf = require('crypto').createHash('md5').update(Buffer.from(data)).digest();
    return bufferToArrayBuffer(buf);
}

exports.md5 = md5;
