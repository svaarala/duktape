/*
 *  Prepare, run, and interpret results of a C/ECMAScript testcase.
 */

'use strict';

const { readFileUtf8, writeFileUtf8, pathJoin, dirname, basename, mkdir, createTempDir } = require('../../util/fs');
const { prepareTestcase } = require('./prep_test');
const { getNowMillis } = require('../../util/time');
const { assert } = require('../../util/assert');
const { createBareObject } = require('../../util/bare');
const { logDebug, logInfo } = require('../../util/logging');
const { readTestcase } = require('./read_testcase');
const { parseKnownIssues } = require('./known_issues');
const { compileDukCommandCached, compileDukLibraryCached } = require('./compile');
const { executeTestcase } = require('./execute');
const { analyzeTestcaseResult } = require('./analyze_result');

async function runTestcase({ engineExePath, repoDirectory, testcaseFilename, knownIssues, ignoreExpect, ignoreSkip, polyfillFilenames, uglifyJsExePath, uglifyJs2ExePath, closureJarPath, dukCommandFilename, dukLibraryFilename, testRunState, runCount, runSleep, sleepMultiplier, sleepAdder, sleepMinimum, numThreads }) {
    var testcaseFilename = assert(testcaseFilename);
    var includeDirectory = pathJoin(repoDirectory, 'tests', 'ecmascript');
    var prepDirectory;
    var testResult;
    var cmd;

    // Create a temporary directory for running the test.
    var tempDirectory = createTempDir({});

    // Read testcase, scan metadata and expected result.
    var {
        testcaseSource,
        testcaseName,
        testcaseMetadata,
        testcaseExpect,
        testcaseType
    } = readTestcase({ testcaseFilename });

    // Prepare runnable testcase by injecting a C or ECMAScript test framework
    // and processing any @include files.
    var { preparedSource } = prepareTestcase({
        testcaseType,
        testcaseSource,
        testcaseMetadata,
        repoDirectory,
        includeDirectory,
        polyfillFilenames,
        promiseHack: true,
        tryCatchWrapper: true,
        tryCatchRethrow: true,
        uglifyJsExePath,
        uglifyJs2ExePath,
        closureJarPath
    });
    var preparedFilename = pathJoin(tempDirectory, testcaseName);
    writeFileUtf8(preparedFilename, preparedSource);

    // Initialize result object.  Keys should all be snake_case.
    testResult = createBareObject({
        testcase_file: testcaseFilename,
        testcase_name: testcaseName,
        //expect: testcaseExpect,
        metadata: testcaseMetadata,
        skipped: false,
        success: null,
        errors: []
    });

    // Skipped testcase?
    if (testcaseMetadata.skip && !ignoreSkip) {
        testResult.skipped = true;
        return testResult;
    }

    // Compile Duktape or libduktape if necessary.  Sources may need to be prepared
    // or compiled multiple times if test cases have forced Duktape options.  Compiled
    // binaries and libraries are cached.
    if (engineExePath && testcaseType === 'ecmascript') {
        dukCommandFilename = engineExePath;
        prepDirectory = void 0; // not available, not needed for ECMAScript tests
    } else if (!dukCommandFilename && testcaseType === 'ecmascript') {
        ({ dukCommandFilename, prepDirectory } = await compileDukCommandCached({ repoDirectory, tempDirectory, testcaseMetadata, testRunState }));
    } else if (!dukLibraryFilename && testcaseType === 'c') {
        ({ dukLibraryFilename, prepDirectory } = await compileDukLibraryCached({ repoDirectory, tempDirectory, testcaseMetadata, testRunState }));
    }

    // Execute testcase.
    var { execResult, durations, sleepTimes } = await executeTestcase({ testcaseType, testcaseName, dukCommandFilename, dukLibraryFilename, prepDirectory, preparedFilename, tempDirectory, runCount, runSleep, sleepMultiplier, sleepAdder, sleepMinimum });

    // Test result analysis.
    var { analysisResult } = analyzeTestcaseResult({ execResult, testcaseMetadata, testcaseExpect, ignoreExpect, knownIssues });
    logDebug('testcase analysis result:', JSON.stringify(analysisResult));

    testResult.success = analysisResult.success;
    testResult.known_issue = analysisResult.knownIssue;
    testResult.duration = execResult.endTime - execResult.startTime;
    testResult.durations = {
        values: durations.values,
        average: durations.average,
        minimum: durations.minimum,
        maximum: durations.maximum,
        standard_deviation: durations.standardDeviation // Convert to snake_case for output.
    };
    testResult.sleep_times = sleepTimes;
    testResult.run_count = runCount;
    testResult.run_sleep = runSleep;
    testResult.sleep_multiplier = sleepMultiplier;
    testResult.sleep_adder = sleepAdder;
    testResult.sleep_minimum = sleepMinimum;
    testResult.num_threads = numThreads; // Not test related but good to know for e.g. performance tests.

    return testResult;
}

async function runParallel({ jobs, numThreads, progressCallback, logFileCallback }) {
    var results = [];
    var states = [];
    var timers = [];
    var jobIndex = 0;
    var completed = 0;
    var startTime = getNowMillis();
    var endTime;
    var duration;
    var prevProgressTime = 0;
    var prevProgressPercentage = -100;
    var progressMinInterval = 10e3;
    var progressMinPercentage = 5;

    function getJob() {
        if (jobIndex < jobs.length) {
            let idx = jobIndex++;
            return [jobs[idx], idx];
        }
        return [null, null];
    }

    function createStateList() {
        return states.map((st) => st);
    }

    function progressCheck(force) {
        if (typeof progressCallback === 'function') {
            let now = getNowMillis();
            let rate = completed / ((now - startTime) / 1000); // jobs/sec
            let etaSeconds = Math.floor((jobs.length - completed) / rate);
            let percentage = Math.floor(completed / jobs.length * 100);
            if (!force) {
                if (now - prevProgressTime < progressMinInterval) {
                    // Don't report too often.
                    return;
                }
                if (percentage < 90 && (percentage - prevProgressPercentage < progressMinPercentage)) {
                    // Don't report small percentage increments, except after 90%.
                    return;
                }
            }
            prevProgressTime = now;
            prevProgressPercentage = percentage;
            progressCallback({ percentage, etaSeconds, states: createStateList() });
        }
    }

    function logFileCheck(doc) {
        if (typeof logFileCallback === 'function') {
            logFileCallback(doc);
        }
    }

    while (states.length < jobs.length) {
        states.push('waiting');
    }

    progressCheck(true);
    while (timers.length < numThreads) {
        let timerIndex = timers.length;
        timers.push(new Promise((resolve, reject) => {
            function timerFunc() {
                let [job, idx] = getJob();
                if (!job) {
                    resolve();
                    return;
                }
                states[idx] = 'running';
                Promise.resolve().then(() => {
                    return job();
                }).then((v) => {
                    results[idx] = v;
                    if (v.skipped) {
                        states[idx] = 'skipped';
                    } else if (v.knownIssue) {
                        states[idx] = 'known';
                    } else {
                        states[idx] = (v.success === true ? 'success' : 'failure');
                    }
                }, (e) => {
                    results[idx] = { success: false, error: e };
                    states[idx] = 'failure';
                }).then(() => {
                    logFileCheck(results[idx]);
                    completed++;
                    progressCheck(false);
                    setTimeout(timerFunc, 0);
                });
            }
            let timerId = setTimeout(timerFunc, 0);
        }));
    }
    await Promise.all(timers);
    endTime = getNowMillis();
    duration = endTime - startTime;
    progressCheck(true);
    return { results, startTime, endTime, duration };
}

function computeFileContentHashByte(fn) {
    let data = readFileUtf8(fn);
    let hash = 0;
    for (let i = 0; i < data.length; i++) {
        hash = (hash + data.charCodeAt(i)) & 0xff;
    }
    return hash;
}

async function runMultipleTests({ engineExePath, repoDirectory, knownIssuesDirectory, filenames, progressCallback, logFileCallback, numThreads = 8, ignoreSkip, polyfillFilenames, uglifyJsExePath, uglifyJs2ExePath, closureJarPath, testHashMin, testHashMax, runCount = 1, runSleep = false, sleepMultiplier = 2.0, sleepAdder = 1.0, sleepMinimum = 1.0 }) {
    // For compile caching and other global run state.
    var testRunState = Object.create(null);
    testRunState.dukPrepareCount = 0;
    testRunState.dukCommandCount = 0;
    testRunState.dukLibraryCount = 0;
    testRunState.dukPrepareCache = createBareObject({});
    testRunState.dukCommandCache = createBareObject({});
    testRunState.dukLibraryCache = createBareObject({});

    let knownIssues = parseKnownIssues({ knownIssuesDirectory });
    logDebug(knownIssues.length, 'known issues');

    var filenames = filenames.filter((fn) => fn.endsWith('.js') || fn.endsWith('.c'));
    if (typeof testHashMin === 'number' && typeof testHashMax === 'number') {
        filenames = filenames.filter((fn) => { let h = computeFileContentHashByte(fn); return h >= testHashMin && h <= testHashMax; });
    }

    var jobs = filenames.map((fn) => {
        return () => {
            return runTestcase({
                engineExePath,
                repoDirectory,
                testcaseFilename: fn,
                knownIssues,
                ignoreSkip,
                polyfillFilenames,
                uglifyJsExePath,
                uglifyJs2ExePath,
                closureJarPath,
                testRunState,
                runCount,
                runSleep,
                sleepMultiplier,
                sleepAdder,
                sleepMinimum,
                numThreads
            });
        };
    });
    var res = await runParallel({
        jobs,
        numThreads,
        progressCallback,
        logFileCallback
    });
    logDebug('runParallel done');
    return res;
}

exports.runMultipleTests = runMultipleTests;
