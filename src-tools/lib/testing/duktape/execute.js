'use strict';

const { asyncExecStdoutUtf8 } = require('../../util/exec');
const { getNowMillis } = require('../../util/time');
const { assert } = require('../../util/assert');
const { compileCTestcase } = require('./compile');
const { logDebug, logInfo } = require('../../util/logging');

async function executeEcmascriptTestcase({ dukCommandFilename, preparedFilename }) {
    var stdout;
    var startTime;
    var endTime;
    var execError;
    var cmd = [];

    cmd.push(assert(dukCommandFilename));
    cmd.push(assert(preparedFilename));

    startTime = getNowMillis();
    try {
        ({ stdout } = await asyncExecStdoutUtf8(cmd, {}));
        endTime = getNowMillis();
    } catch (e) {
        endTime = getNowMillis();
        execError = String(e);
    }

    return { stdout, startTime, endTime, execError };
}

async function executeCTestcase({ cExeFilename }) {
    var startTime;
    var endTime;
    var stdout;
    var cmd = [];

    cmd.push(cExeFilename);

    startTime = getNowMillis();
    try {
        ({ stdout } = await asyncExecStdoutUtf8(cmd, {}));
        endTime = getNowMillis();
    } catch (e) {
        endTime = getNowMillis();
    }

    return { stdout, startTime, endTime };
}

async function executeTestcase({ testcaseType, dukCommandFilename, dukLibraryFilename, prepDirectory, preparedFilename, tempDirectory }) {
    var count = 1;
    var results = [];
    var durations = [];

    // For C test cases, compile the test case once before the test execution loop.
    var cExeFilename;
    if (testcaseType === 'c') {
        ({ cExeFilename } = await compileCTestcase({
            prepDirectory,
            tempDirectory,
            preparedFilename,
            dukLibraryFilename
        }));
    }
    logDebug(cExeFilename);

    // Execute test one or more times.  Keep track of timing statistics.
    for (let i = 0; i < count; i++) {
        let execResult;

        if (testcaseType === 'ecmascript') {
            execResult = await executeEcmascriptTestcase({
                dukCommandFilename,
                preparedFilename
            });
        } else if (testcaseType === 'c') {
            execResult = await executeCTestcase({ cExeFilename });
        } else {
            throw new TypeError('internal error');
        }

        let duration = execResult.endTime - execResult.startTime;
        let sleepTime = duration * 2.0;

        durations.push(execResult.endTime - execResult.startTime);
        results.push(execResult);
    }

    return {
        execResults: results,
        execDurations: durations,
        execResult: results[0]
    };
}

exports.executeTestcase = executeTestcase;
