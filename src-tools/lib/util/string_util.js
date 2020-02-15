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
