// Misc utils for handling strings interpreted as bytes (U+0000 to U+00FF),
// sometimes referred to as "byte string" or "bstr".
//
// https://developer.mozilla.org/en-US/docs/Web/API/DOMString/Binary
// https://developer.mozilla.org/en-US/docs/Web/API/WindowOrWorkerGlobalScope/btoa

'use strict';

const { assert } = require('./assert.js');

function bstrToArray(s) {
    var res = [];
    s.split('').forEach((c) => {
        let o = c.charCodeAt(0);
        if (o >= 0x100) {
            throw new TypeError('invalid codepoint in input string: ' + o);
        }
        res.push(o);
    });
    return res;
}
exports.bstrToArray = bstrToArray;

function bstrToUint8Array(s) {
    return new Uint8Array(bstrToArray(s));
}
exports.bstrToUint8Array = bstrToUint8Array;

function uint8ArrayToBstr(u8) {
    return String.fromCharCode.apply(String, u8);
}
exports.uint8ArrayToBstr = uint8ArrayToBstr;

function arrayToBstr(arr) {
    return String.fromCharCode.apply(String, arr);
}
exports.arrayToBstr = arrayToBstr;

function isBstr(s) {
    if (typeof s !== 'string') {
        return false;
    }
    for (let i = 0; i < s.length; i++) {
        if (s.charCodeAt(i) >= 0x100) {
            return false;
        }
    }
    return true;
}
exports.isBstr = isBstr;

function assertBstr(s) {
    assert(isBstr(s));
}
exports.assertBstr = assertBstr;
