'use strict';

const { asyncExecStdoutUtf8 } = require('../../util/exec');
const { getNowMillis } = require('../../util/time');
const { assert } = require('../../util/assert');
const { sleep } = require('../../util/sleep');
const { logDebug, logInfo } = require('../../util/logging');
const { compileCTestcase } = require('./compile');

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

// Execute a C testcase (API tests).  This is much trickier than ECMAScript
// testcases because we must also compile the code.  Two methods of execution
// are supported:
//
//   1. Given configured Duktape sources, compile Duktape and testcase
//      together for execution.
//   2. Given configured Duktape sources and a precompiled libduktape*.so,
//      compile testcase and link against the precompiled library.  This
//      is much faster.

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

function computeStats(values) {
    let average, minimum, maximum;
    let variance, standardDeviation;

    if (values.length > 0) {
        let sum = 0;
        for (let v of values) {
            sum += v;
        }
        minimum = Math.min.apply(null, values);
        maximum = Math.max.apply(null, values);
        average = sum / values.length;

        let varsum = 0;
        for (let v of values) {
            varsum += (v - average) ** 2;
        }
        variance = varsum / values.length;
        standardDeviation = Math.sqrt(variance);
    }

    return { average, minimum, maximum, variance, standardDeviation };
}

async function executeTestcase({ testcaseType, testcaseName, dukCommandFilename, dukLibraryFilename, prepDirectory, preparedFilename, tempDirectory, runCount = 1, runSleep = false, sleepMultiplier = 2.0, sleepAdder = 1.0, sleepMinimum = 1.0 }) {
    var results = [];
    var durations = [];
    var sleepTimes = [];

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
    for (let i = 0; i < runCount; i++) {
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

        let duration = (execResult.endTime - execResult.startTime) / 1000;

        durations.push(duration);
        results.push(execResult);

        if (runSleep) {
            let sleepTime = Math.max(sleepMultiplier * duration + sleepAdder, sleepMinimum) * 1000;
            logDebug('sleeping', sleepTime, 'milliseconds');
            await sleep(sleepTime);
            sleepTimes.push(sleepTime);
        }
    }

    let { average, minimum, maximum, standardDeviation } = computeStats(durations);

    let retval = {
        execResults: results,
        execResult: results[0],

        sleepTimes,

        durations: { values: durations, average, minimum, maximum, standardDeviation },
        duration: minimum // easy consumption
    };

    return retval;
}

exports.executeTestcase = executeTestcase;
