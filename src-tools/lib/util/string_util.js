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

// Check if string is an "array index" in ECMAScript terms.
function stringIsArridx(x) {
    if (typeof x !== 'string') {
        throw new TypeError('invalid argument');
    }
    if (/-?[0-9]+/.test(x)) {
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
