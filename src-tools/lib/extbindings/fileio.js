'use strict';

const global = new Function('return this')();
const { assert } = require('../util/assert');
const { bufferToUint8Array } = require('../util/buffer');
const { utf8Uint8ArrayToString, stringToUtf8Uint8Array } = require('./utf8');

function readFile(filename) {
    assert(typeof filename === 'string');

    if (typeof global.readFile === 'function') {
        // Duktape
        let data = Object(global.readFile(filename));
        assert(data instanceof Uint8Array);
        return data;
    } else {
        // Node.js
        const fs = require('fs');
        let data = fs.readFileSync(filename);
        assert(data instanceof Buffer);
        data = bufferToUint8Array(data);
        assert(data instanceof Uint8Array);
        return data;
    }
}
exports.readFile = readFile;

function readFileUtf8(filename) {
    assert(typeof filename === 'string');
    var u8 = readFile(filename);
    assert(u8 instanceof Uint8Array);

    return utf8Uint8ArrayToString(u8);
}
exports.readFileUtf8 = readFileUtf8;

function writeFile(filename, data) {
    assert(typeof filename === 'string');
    assert(data instanceof Uint8Array);

    if (typeof global.writeFile === 'function') {
        // Duktape
        global.writeFile(filename, data);
    } else {
        // Node.js
        const fs = require('fs');
        fs.writeFileSync(filename, data);
    }
}
exports.writeFile = writeFile;

function writeFileUtf8(filename, data) {
    assert(typeof filename === 'string');
    assert(typeof data === 'string');

    let u8 = stringToUtf8Uint8Array(data);
    assert(u8 instanceof Uint8Array);
    writeFile(filename, u8);
}
exports.writeFileUtf8 = writeFileUtf8;
