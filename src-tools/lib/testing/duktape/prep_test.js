/*
 *  Prepare an ECMAScript or API testcase file for execution.
 */

'use strict';

const { assert } = require('../../util/assert');
const { minifyBestAvailable } = require('./minify');
const { ecmascriptTestFrameworkSource } = require('./ecma_test_framework');
const { cTestFrameworkSource, cTestcaseMain } = require('./c_test_framework');
const { readFileUtf8, pathJoin, dirname } = require('../../util/fs');
const { parseTestcaseMetadata } = require('./parse_test_meta');

function prepareCTestcase({ sourceString }) {
    assert(typeof sourceString === 'string');

    // Prepend some defines to the testcase, and a main() function.
    let result = cTestFrameworkSource + sourceString + '\n' + cTestcaseMain;

    return result;
}
exports.prepareCTestcase = prepareCTestcase;

function prepareEcmascriptTestcase({ sourceString, includeDirectory, testcaseMetadata, polyfillFilenames, promiseHack, tryCatchWrapper, tryCatchRethrow, uglifyJsExePath, uglifyJs2ExePath, closureJarPath }) {
    assert(typeof sourceString === 'string');
    var meta = testcaseMetadata || {};
    var parts = [];
    var sourceWithIncludes;

    function minify(sourceString) {
        let { result } = minifyBestAvailable({
            sourceString,
            uglifyJsExePath,
            uglifyJs2ExePath,
            closureJarPath
        });
        return result;
    }

    // Process include files.  Replace each include with a one-line
    // minified version of the include file, thus preserving line
    // numbers.
    sourceWithIncludes = sourceString.replace(/^\/\*@include\s+(.*?)\s*@\*\/$/gm, (m, a) => {
        assert(typeof includeDirectory === 'string');
        let fn = pathJoin(includeDirectory, a);
        let data = readFileUtf8(fn);
        return '/* Included: ' + a + ' -> ' + fn + ' */ ' + minify(data);
    });

    // If the testcase needs to run strict program level code, prepend
    // a 'use strict' declaration.
    if (meta.use_strict) {
        parts.push("/* FORCE STRICT START */ 'use strict'; /* FORCE STRICT END */ ");
    }

    // Inject shared engine prefix and a possible try-catch wrapper,
    // without disturbing line numbers.
    parts.push(minify(ecmascriptTestFrameworkSource), ' ');
    if (tryCatchWrapper) {
        parts.push('/* TRY-CATCH WRAPPER START */ try { /* TRY-CATCH WRAPPER END */ ');
    }

    // Insert polyfills.
    if (Array.isArray(polyfillFilenames)) {
        for (let fn of polyfillFilenames) {
            let data = readFileUtf8(fn);
            parts.push('/* START POLYFILL: ' + fn + ' */ ', minify(data), ' /* END POLYFILL: ' + fn + ' */ ');
        }
    }

    // Prepared source code, still preserving line numbers.
    parts.push(sourceWithIncludes);

    // Promise hack; run job queue to completion at end of file.
    if (promiseHack) {
        parts.push("/* START PROMISE HACK */ if (typeof Promise === 'function' && typeof Promise.runQueue === 'function') { Promise.runQueue(); } /* END PROMISE HACK */ ");
    }

    // End of try-catch wrapper.
    if (tryCatchWrapper) {
        parts.push('/* TRY-CATCH WRAPPER START */ } catch (wrapperError) { console.log(wrapperError.stack || wrapperError);');
        if (tryCatchRethrow) {
            parts.push(' throw(wrapperError);');
        } else {
            parts.push(' /*tryCatchRethrow disabled, eat error silently*/');
        }
        parts.push(' } /* TRY-CATCH WRAPPER END */');
    }

    return parts.join('');
}
exports.prepareEcmascriptTestcase = prepareEcmascriptTestcase;

function prepareTestcase({ testcaseFilename, testcaseType, testcaseSource, testcaseMetadata, includeDirectory, polyfillFilenames, promiseHack=true, tryCatchWrapper=true, tryCatchRethrow=false, uglifyJsExePath, uglifyJs2ExePath, closureJarPath }) {
    if (testcaseFilename) {
        assert(typeof testcaseFilename === 'string');
        if (testcaseFilename.endsWith('.js')) {
            testcaseType = testcaseType || 'ecmascript';
        } else if (testcaseFilename.endsWith('.c')) {
            testcaseType = testcaseType || 'c';
        } else {
            throw new TypeError('cannot determine testcase type for file: ' + testcaseFilename);
        }
        testcaseSource = readFileUtf8(testcaseFilename);
        includeDirectory = includeDirectory || dirname(testcaseFilename);
    } else if (testcaseSource && testcaseType) {
        // Keep as is.
        assert(typeof testcaseSource === 'string');
        assert(typeof testcaseType === 'string');
    } else {
        throw new TypeError('must provide testcaseFilename or testcaseSource and testcaseType');
    }

    var preparedSource;
    testcaseMetadata = testcaseMetadata || parseTestcaseMetadata({ sourceString: testcaseSource });
    if (testcaseType === 'ecmascript') {
        preparedSource = prepareEcmascriptTestcase({
            sourceString: testcaseSource,
            includeDirectory,
            testcaseMetadata,
            polyfillFilenames,
            promiseHack,
            tryCatchWrapper,
            tryCatchRethrow,
            uglifyJsExePath,
            uglifyJs2ExePath,
            closureJarPath
        });
    } else if (testcaseType === 'c') {
        preparedSource = prepareCTestcase({
            sourceString: testcaseSource,
            includeDirectory,
            testcaseMetadata
        });
    } else {
        throw new TypeError('unknown testcase type: ' + testcaseType);
    }

    return { preparedSource, testcaseMetadata, testcaseSource, testcaseFilename, testcaseType, includeDirectory };
}
exports.prepareTestcase = prepareTestcase;
