'use strict';

function stripLastNewline(x) {
    if (x.endsWith('\n')) {
        x = x.substring(0, x.length - 1);
    }
    return x;
}
exports.stripLastNewline = stripLastNewline;

function normalizeNewlines(x) {
    return x.replace('\r\n', '\n');
}
exports.normalizeNewlines = normalizeNewlines;

// Check if string is an "array index" in ECMAScript terms.  For zero,
// only '0' is an arridx, '-0' and '+0' are not.
function stringIsArridx(x) {
    if (typeof x !== 'string') {
        throw new TypeError('invalid argument');
    }
    if (x === '0') {
        return true;
    }
    if (/[1-9][0-9]*/.test(x)) {
        let ival = Math.floor(Number(x));
        if (String(ival) !== x) {
            return false;
        }
        if (ival >= 0 && ival <= 0xfffffffe) {
            return true;
        }
    }
    return false;
}
exports.stringIsArridx = stringIsArridx;

function stringIsHiddenSymbol(v) {
    if (v.length >= 1) {
        let x = v.charCodeAt(0);
        return (x == 0x82) || (x == 0xff);
    }
    return false;
}
exports.stringIsHiddenSymbol = stringIsHiddenSymbol;

function stringIsAnySymbol(v) {
    if (v.length >= 1) {
        let x = v.charCodeAt(0);
        return (x == 0x80) || (x == 0x81) || (x == 0x82) || (x == 0xff);
    }
    return false;
}
exports.stringIsAnySymbol = stringIsAnySymbol;
