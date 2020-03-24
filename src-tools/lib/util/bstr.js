// Misc utils for handling strings interpreted as bytes (U+0000 to U+00FF),
// sometimes referred to as "byte string" or "bstr".
//
// https://developer.mozilla.org/en-US/docs/Web/API/DOMString/Binary
// https://developer.mozilla.org/en-US/docs/Web/API/WindowOrWorkerGlobalScope/btoa

'use strict';

const { assert } = require('./assert');

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

function validateStringsAreBstrRecursive(doc) {
    function visit(x) {
        if (typeof x === 'string') {
            if (!isBstr(x)) {
                throw new TypeError('found non-bstr string (strings must consist of U+0000-U+00FF identified as encoded bytes): ' + JSON.stringify(x));
            }
        } else if (typeof x === 'boolean') {
            // No validation
        } else if (typeof x === 'number') {
            // No validation
        } else if (x === void 0 || x === null) {
            // No validation
        } else if (typeof x === 'object' && x !== null && Array.isArray(x)) {
            for (let i = 0; i < x.length; i++) {
                visit(x[i]);
            }
        } else if (typeof x === 'object' && x !== null) {
            for (let k of Object.keys(x).sort()) {
                visit(k);
                visit(x[k]);
            }
        } else {
            // Unexpected type, nothing to validate, ignore.
        }
    }
    visit(doc);
}
exports.validateStringsAreBstrRecursive = validateStringsAreBstrRecursive;

// Convert all strings in a document to Uint8Array.
function recursiveBstrToUint8Array(doc) {
    function f(x) {
        if (typeof x === 'string') {
            return bstrToUint8Array(x);
        } else if (typeof x === 'object' && x !== null && Array.isArray(x)) {
            let res = [];
            for (let i = 0; i < x.length; i++) {
                res.push(f(x[i]));
            }
            return res;
        } else if (typeof x === 'object' && x !== null) {
            let res = {};
            for (let k in x) {
                // Python tooling encoded key too; this doesn't work well in JS.
                //res[f(k)] = f(x[k]);
                res[k] = f(x[k]);
            }
            return res;
        } else {
            return x;
        }
    }
    return f(doc);
}
exports.recursiveBstrToUint8Array = recursiveBstrToUint8Array;

// Convert all strings in an object to from Uint8Array to bstr.
function recursiveUint8ArrayToBstr(doc) {
    function f(x) {
        if (typeof x === 'object' && x !== null && x instanceof Uint8Array) {
            return uint8ArrayToBstr(x);
        } else if (typeof x === 'object' && x !== null && Array.isArray(x)) {
            let res = [];
            for (let i = 0; i < x.length; i++) {
                res.push(f(x[i]));
            }
            return res;
        } else if (typeof x === 'object' && x !== null) {
            let res = {};
            for (let k in x) {
                //res[f(k)] = f(x[k]);
                res[k] = f(x[k]);
            }
            return res;
        } else {
            return x;
        }
    }

    return f(doc);
}
exports.recursiveUint8ArrayToBstr = recursiveUint8ArrayToBstr;
