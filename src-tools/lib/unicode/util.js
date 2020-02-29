'use strict';

const { hexEncodeUpper } = require('../util/hex.js');

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
