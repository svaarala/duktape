'use strict';

const global = new Function('return this')();
const { assert } = require('../util/assert');
const { bufferToUint8Array } = require('../util/buffer');

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

    if (typeof TextEncoder === 'function') {
        // Duktape or recent Node.js
        let u8 = new TextEncoder().encode(data);
        assert(u8 instanceof Uint8Array);
        writeFile(filename, u8);
    } else {
        // Older Node.js
        assert(typeof Buffer === 'function');
        let u8 = bufferToUint8Array(Buffer.from(data, 'utf-8'));
        assert(u8 instanceof Uint8Array);
        writeFile(filename, u8);
    }
}
exports.writeFileUtf8 = writeFileUtf8;
