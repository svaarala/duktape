'use strict';

const { assert } = require('../../util/assert');
const { readFileUtf8, writeFileUtf8, createTempDir, pathJoin } = require('../../util/fs');
const { execStdoutUtf8 } = require('../../util/exec');
const { logDebug } = require('../../util/logging');

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

function minifyHelper({ sourceString, cmd, tmpInFn, tmpOutFn }) {
    writeFileUtf8(tmpInFn, sourceString);
    writeFileUtf8(tmpOutFn, '');

    logDebug('execute minify:', cmd);
    let stdout = execStdoutUtf8(cmd, {});

    let result = readFileUtf8(tmpOutFn);
    return { result };
}

function setupTemps() {
    let tempDirectory = createTempDir({});
    let tmpInFn = pathJoin(tempDirectory, 'minify-input');
    let tmpOutFn = pathJoin(tempDirectory, 'minify-output');
    return { tmpInFn, tmpOutFn };
}

function minifyClosure({ sourceString, closureJarPath }) {
    assert(typeof sourceString === 'string');
    assert(typeof closureJarPath === 'string');
    let { tmpInFn, tmpOutFn } = setupTemps();

    return minifyHelper({
        sourceString,
        cmd: ['java', '-jar', closureJarPath, '--js_output_file', tmpOutFn, tmpInFn],
        tmpInFn,
        tmpOutFn
    });
}

function minifyUglifyJs({ sourceString, uglifyJsExePath }) {
    assert(typeof sourceString === 'string');
    assert(typeof uglifyJsExePath === 'string');
    let { tmpInFn, tmpOutFn } = setupTemps();

    return minifyHelper({
        sourceString,
        cmd: [uglifyJsExePath, '-o', tmpOutFn, tmpInFn],
        tmpInFn,
        tmpOutFn
    });
}

function minifyUglifyJs2({ sourceString, uglifyJs2ExePath }) {
    assert(typeof sourceString === 'string');
    assert(typeof uglifyJs2ExePath === 'string');
    let { tmpInFn, tmpOutFn } = setupTemps();

    return minifyHelper({
        sourceString,
        cmd: [uglifyJs2ExePath, '-o', tmpOutFn, tmpInFn],
        tmpInFn,
        tmpOutFn
    });
}

function minifyBestAvailable({ sourceString, uglifyJsExePath, uglifyJs2ExePath, closureJarPath }) {
    if (typeof closureJarPath === 'string') {
        logDebug('minify with closure');
        return minifyClosure({ sourceString, closureJarPath });
    } else if (typeof uglifyJsExePath === 'string') {
        logDebug('minify with uglifyjs');
        return minifyUglifyJs({ sourceString, uglifyJsExePath });
    } else if (typeof uglifyJs2ExePath === 'string') {
        logDebug('minify with uglifyjs2');
        return minifyUglifyJs2({ sourceString, uglifyJs2ExePath });
    } else {
        logDebug('minify with dummy');
        return minifyDummy({ sourceString });
    }
    assert(false);
}

exports.minifyDummy = minifyDummy;
exports.minifyClosure = minifyClosure;
exports.minifyUglifyJs = minifyUglifyJs;
exports.minifyUglifyJs2 = minifyUglifyJs2;
exports.minifyBestAvailable = minifyBestAvailable;
