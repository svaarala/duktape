'use strict';

const { assert } = require('../../util/assert');

// Parse an expect block from sourceString file.  Initial newline is excluded but
// newline on last expect line is included.  If no expect blocks are found,
// returns undefined, i.e. no expect value (for example, a perf test).
function parseTestcaseExpectedResult({ sourceString }) {
    assert(typeof sourceString === 'string');
    var res = [];
    var foundExpectBlocks = false;
    var dummy = sourceString.replace(/\\*===\n^((?:.|\r|\n)*?)^===\*\/|\/\/>(.*?)$/gm, (m, a, b) => {
        foundExpectBlocks = true;
        if (a !== void 0) {
            res.push(a);
        } else if (b !== void 0) {
            res.push(b + '\n');  // single-line shorthand
        } else {
            assert(false);
        }
    });
    void dummy;
    return foundExpectBlocks ? res.join('') : void 0;
}
exports.parseTestcaseExpectedResult = parseTestcaseExpectedResult;
