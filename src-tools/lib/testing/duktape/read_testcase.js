'use strict';

const { readFileUtf8, basename } = require('../../util/fs');
const { parseTestcaseMetadata } = require('./parse_test_meta');
const { parseTestcaseExpectedResult } = require('./parse_test_expect');

function readAndNormalizeTestcase(fn) {
    var data = readFileUtf8(fn).replace('\r', '');
    return data;
}

function readTestcase({ testcaseFilename }) {
    var testcaseSource = readAndNormalizeTestcase(testcaseFilename);
    var testcaseName = basename(testcaseFilename);
    var testcaseMetadata = parseTestcaseMetadata({ sourceString: testcaseSource });
    var testcaseExpect = parseTestcaseExpectedResult({ sourceString: testcaseSource });
    var testcaseType;
    if (testcaseFilename.endsWith('.js')) {
        testcaseType = 'ecmascript';
    } else if (testcaseFilename.endsWith('.c')) {
        testcaseType = 'c';
    } else {
        throw new TypeError('cannot determine testcase type: ' + testcaseFilename);
    }

    return {
        testcaseSource,
        testcaseName,
        testcaseMetadata,
        testcaseExpect,
        testcaseType
    };
}

exports.readTestcase = readTestcase;
