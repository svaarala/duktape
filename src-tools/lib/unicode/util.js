'use strict';

// Convert codepoint sequence (Array) to a String.
function codepointSequenceToString(seq) {
    return String.fromCharCode.apply(String, seq);
}
exports.codepointSequenceToString = codepointSequenceToString;
