'use strict';

const { findKnownIssue } = require('./known_issues');
const { logDebug } = require('../../util/logging');

// Analyze testcase execution result against testcase expect, testcase metadata,
// and known issues.
function analyzeTestcaseResult({ execResult, testcaseMetadata, testcaseExpect, ignoreExpect, knownIssues }) {
    var res = {};
    res.stdoutExpect = testcaseExpect;
    res.stdoutActual = execResult.stdout ?? '';

    if (execResult.execError) {
        if (testcaseMetadata.intended_uncaught) {
            res.success = true;
        } else {
            res.success = false;
        }
    } else {
        if (testcaseMetadata.intended_uncaught) {
            res.success = false;
        } else if (ignoreExpect) {
            res.success = true;
        } else if (typeof testcaseExpect !== 'string') {
            // No expect, e.g. perf test.
            res.success = true;
        } else if (execResult.stdout === testcaseExpect) {
            res.success = true;
        } else {
            // Expect does not match actual output.  Check if it's a known issue.
            let known = findKnownIssue({ knownIssues, stdoutActual: execResult.stdout });
            if (known) {
                logDebug('known issue:', known.filename);
                res.success = true;
                res.knownIssue = known;
            } else {
                res.success = false;
            }
        }
    }

    return { analysisResult: res };
}

exports.analyzeTestcaseResult = analyzeTestcaseResult;
