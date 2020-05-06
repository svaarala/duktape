'use strict';

const { hexEncodeUpper } = require('../util/hex');

// Convert codepoint sequence (Array) to a String.
function codepointSequenceToString(seq) {
    return String.fromCharCode.apply(String, seq);
}
exports.codepointSequenceToString = codepointSequenceToString;

// Convert a codepoint sequence to ranges.
function codepointSequenceToRanges(seq) {
    var start, end;
    var res = [];
    for (let i = 0; i < seq.length; i++) {
        let cp = seq[i];
        if (start === void 0) {
            start = cp;
            end = cp;
        } else if (end + 1 === cp) {
            end = cp;
        } else {
            res.push([ start, end ]);
            start = cp;
            end = cp;
        }
    }
    if (start !== void 0) {
        res.push([ start, end ]);
    }
    return res;
}
exports.codepointSequenceToRanges = codepointSequenceToRanges;

// Convert a list of ranges ([ [start,end], [start,end], ... ]) to
// a list of printable range strings.
function rangesToPrettyRanges(ranges) {
    var prettyRanges = ranges.map((r) => {
        if (r[0] === r[1]) {
            return hexEncodeUpper(r[0], 4);
        } else {
            return hexEncodeUpper(r[0], 4) + '-' + hexEncodeUpper(r[1], 4);
        }
    });
    return prettyRanges;
}
exports.rangesToPrettyRanges = rangesToPrettyRanges;

function rangesToPrettyRangesDump(ranges) {
    return rangesToPrettyRanges(ranges).join('\n') + '\n';
}
exports.rangesToPrettyRangesDump = rangesToPrettyRangesDump;

function rangesToTextBitmapDump(ranges) {
    var bitmap = [];
    while (bitmap.length < 0x110000) {
        bitmap.push('.');
    }
    ranges.forEach((r) => {
        for (let i = r[0]; i <= r[1]; i++) {
            bitmap[i] = 'x';
        }
    });
    bitmap = bitmap.join('');
    var lines = [];
    var empty = '.'.repeat(256);
    var startOmit;
    var last;
    for (let i = 0; i < bitmap.length; i += 256) {
        let line = bitmap.substring(i, i + 256);
        let base = ('00000000' + i.toString(16)).substr(-8);
        if (line === last && i >= 65536) {
            if (startOmit === void 0) {
                startOmit = i;
            }
            continue;
        }
        if (startOmit !== void 0) {
            lines.push('           ' + '| | |');
            startOmit = void 0;
        }
        lines.push(base + ': ' + line);
        last = line;
    }
    if (startOmit !== void 0) {
        lines.push('           ' + '| | |');
        startOmit = void 0;
    }

    return lines.join('\n') + '\n';
}
exports.rangesToTextBitmapDump = rangesToTextBitmapDump;

function dumpUnicodeCategories(cpMap, cats) {
    var res = [];
    Object.getOwnPropertyNames(cats).forEach((gc) => {
        let seq = cats[gc];
        res.push('CATEGORY: ' + gc);
        seq.forEach((cp) => {
            res.push('- ' + hexEncodeUpper(cp, 4) + ': ' + cpMap[cp].name);
        });
        res.push('RANGES:');
        rangesToPrettyRanges(codepointSequenceToRanges(seq)).forEach((str) => {
            res.push('- ' + str);
        });
    });
    return res.join('\n') + '\n';
}
exports.dumpUnicodeCategories = dumpUnicodeCategories;
