'use strict';

const { createBareObject } = require('../util/bare');
const { runMultipleTests } = require('../testing/duktape/run_test');
const { findTestCasesSync } = require('../testing/duktape/find_testcases');
const { pathJoin, writeFileUtf8 } = require('../util/fs');
const { estimateCoreCount } = require('../util/core_count');
const { logInfo, logDebug } = require('../util/logging');

const defaultNumThreads = 8;

const runTestsCommandSpec = createBareObject({
    description: 'Run Duktape testcases(s)',
    options: createBareObject({
        'source-directory': { type: 'path', default: void 0, description: 'Directory with raw input sources (defaulted based on script path)' },
        'num-threads': { short: 'j', type: 'number', default: estimateCoreCount() ?? defaultNumThreads, description: 'Number of threads to use for running tests' },
        'uglifyjs-bin': { type: 'path', default: void 0, description: 'Path to UglifyJS binary for minifying' },
        'test-log-file': { type: 'path', default: void 0, description: 'Path to ndjson test log file' },
        'test-hash-min': { type: 'number', default: void 0, description: 'Minimum value for testcase content hash' },
        'test-hash-max': { type: 'number', default: void 0, description: 'Maximum value (inclusive) for testcase content hash' }
    })
});
exports.runTestsCommandSpec = runTestsCommandSpec;

async function runTestsCommand({ commandOpts, commandPositional }, autoDuktapeRoot) {
    if (!autoDuktapeRoot && opts['source-directory']) {
        autoDuktapeRoot = pathJoin(opts['source-directory'], '..');
    }

    var filenames = findTestCasesSync({ argList: commandPositional, recursive: true });

    var testLogFile = commandOpts['test-log-file'];
    if (typeof testLogFile === 'string') {
        logInfo('writing test log to', testLogFile);
        writeFileUtf8(testLogFile, '');
    }

    const stateMap = { waiting: '.', running: '@', success: '*', failure: '!', skipped: '-', known: 'k' };

    let { results, duration } = await runMultipleTests({
        filenames,
        progressCallback: ({ percentage, etaSeconds, states }) => {
            let chars = states.map((st) => stateMap[st] ?? '?');
            console.log('Progress: ' + Math.floor(percentage) + '%' + (etaSeconds >= 0 ? ' [ETA ' + etaSeconds + 's]' : ''));
            console.log(chars.join('').replace(/.{120}/g, (v) => v + '\n'));
        },
        logFileCallback: (doc) => {
            if (typeof testLogFile === 'string') {
                require('fs').appendFileSync(testLogFile, JSON.stringify(doc) + '\n');
            }
        },
        numThreads: commandOpts['num-threads'] ?? defaultNumThreads,
        polyfillFilenames: [],
        uglifyJs2ExePath: commandOpts['uglifyjs-bin'],
        repoDirectory: autoDuktapeRoot,
        includeDirectory: pathJoin(autoDuktapeRoot, 'tests/ecmascript'),
        knownIssuesDirectory: pathJoin(autoDuktapeRoot, 'tests/knownissues'),
        testHashMin: commandOpts['test-hash-min'] ? +commandOpts['test-hash-min'] : void 0,
        testHashMax: commandOpts['test-hash-max'] ? +commandOpts['test-hash-max'] : void 0
    });

    var success = 0,
        failure = 0,
        skipped = 0,
        total = results.length;
    results.forEach((result) => {
        if (result.success === true) {
            success++;
        } else if (result.success === false) {
            console.log('Failed:', JSON.stringify(result));
            failure++;
        } else {
            skipped++;
        }
    });
    console.log('Success ' + success + '/' + total + ', failure ' + failure + '/' + total +
        ', skipped ' + skipped + '/' + total + ' (' + Math.floor(duration / 1000) + ' seconds)');

    if (failure > 0) {
        throw new TypeError('one or more failed testcases (' + failure + ')');
    }
}

exports.runTestsCommand = runTestsCommand;
