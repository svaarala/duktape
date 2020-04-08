'use strict';

const { assert } = require('./assert');

// Pad input to 'len' characters on the right, return as is if input
// is already longer than 'len'.
function rightPad(x, len) {
    return x + (' ').repeat(Math.max(len - x.length, 0));
}

// Layout input columns (arrays of lines) into lines so that columns are
// autofitted to have the same width.
function layoutTextColumns(columns) {
    var maxlens = [];
    var maxlines = 0;

    columns.forEach((column) => {
        let maxlen = 0;
        column.forEach((x) => { assert(typeof x === 'string'); maxlen = Math.max(maxlen, x.length); });
        maxlens.push(maxlen);
        maxlines = Math.max(maxlines, column.length);
    });

    var lines = [];
    for (let i = 0; i < maxlines; i++) {
        let parts = [];
        for (let j = 0; j < columns.length; j++) {
            parts.push(rightPad(columns[j][i] || '', maxlens[j]));
        }
        lines.push(parts.join('  '));
    }

    return lines;
}
exports.layoutTextColumns = layoutTextColumns;
