'use strict';

const { assert } = require('./assert');
const { assertBstr } = require('./bstr');

// Encode an integer into a C constant.
function cIntEncode(x) {
    assert(typeof x === 'number');
    // XXX: unsigned constants? DUK_I64_CONSTANT for LL?
    if (x > 0x7fffffff || x < -0x80000000) {
        return String(x) + 'LL';
    } else if (x > 0x7fff || x < -0x8000) {
        return String(x) + 'L';
    } else {
        return String(x);
    }
}
exports.cIntEncode = cIntEncode;

// Encode a bstr into a C string constant.  Split the constant at each
// escape sequence to avoid any ambiguity, e.g. 'foo\u00ffbar' =>
// ("foo" "\xff" "bar").  Consecutive hex escapes don't cause ambiguity
// so place them in the same C string part, e.g. 'foo\u00ff\u00febar' =>
// ("foo" "\xff\xfe" "bar").
function cStrEncode(x) {
    assert(typeof x === 'string');
    assertBstr(x);

    // First alternative max matches characters that need quoting.
    // Second alternative max matches characters that don't need quoting.
    var re = /([\u0000-\u001f\u007e-\uffff\u0022\u005c]+)|([^\u0000-\u001f\u007e-\uffff\u0022\u005c]+)/g;

    var parts = [];
    void x.replace(re, (m, a, b) => {
        if (typeof a === 'string') {
            // Consecutive hex escapes cannot cause ambiguity, so place them in
            // the same C string part.
            parts.push('"' + Array.prototype.map.call(a, (x) => {
                return '\\x' + ('00' + x.charCodeAt(0).toString(16)).substr(-2);
            }).join('') + '"');
        } else if (typeof b === 'string') {
            parts.push('"' + b + '"');
        } else {
            throw new TypeError('internal error');
        }
    });

    if (parts.length == 0) {
        return '""';
    } else if (parts.length == 1) {
        return parts[0];
    } else {
        return '(' + parts.join(' ') + ')';
    }
}
exports.cStrEncode = cStrEncode;

function cCommentEscape(x) {
    return x.replace('*', 'x');
}
exports.cCommentEscape = cCommentEscape;

function test() {
    assert(cStrEncode('') === '""');
    assert(cStrEncode('foo') === '"foo"');
    assert(cStrEncode('foo\u0009bar') === '("foo" "\\x09" "bar")');
    assert(cStrEncode('foo\u0022\u005cbar') === '("foo" "\\x22\\x5c" "bar")');
    assert(cStrEncode('foo\u00ff\u00febar\u00a9quux"\\\u0009z') == '("foo" "\\xff\\xfe" "bar" "\\xa9" "quux" "\\x22\\x5c\\x09" "z")');
}
exports.test = test;
