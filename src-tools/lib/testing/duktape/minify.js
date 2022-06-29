'use strict';

const { assert } = require('../../util/assert');
const { readFileUtf8, writeFileUtf8 } = require('../../util/fs');
const { execStdoutUtf8 } = require('../../util/exec');

// Trivial minifier which tries to convert an input into a single line
// source file: (1) replace single line comments with multiline
// comments, (2) replace newlines with spaces.  This works reasonably
// well for input source code containing explicit semicolons.
function minifyDummy({ sourceString }) {
    let src = sourceString;
    if (src.indexOf('\n') < 0) {
        return src;
    }
    src = src.replace(/\/\/(.*?)$/gm, (m, a) => '/* ' + a.trim() + ' */')
        .replace(/\n/g, ' ')
        .trim();
    assert(src.indexOf('\n') < 0);
    return { result: src };
}

function minifyBestAvailable({ sourceString, uglifyJsExePath, uglifyJs2ExePath, closureJarPath }) {
    return minifyDummy({ sourceString });
}

exports.minifyDummy = minifyDummy;
exports.minifyBestAvailable = minifyBestAvailable;
