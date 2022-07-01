'use strict';

const { logDebug, logInfo, logWarn } = require('./lib/util/logging');

// Run as early as possible to warn if Node.js is too old.
function nodejsVersionCheck() {
    const minMajor = 14;

    if (typeof process === 'object' && process !== null &&
        typeof process.version === 'string') {
        var version = process.version;
        var parts = version.replace(/^v/, '').split('.').map(Number);
        logDebug('nodejs version:', parts);
        if (parts.length === 3 && parts[0] < minMajor) {
            logWarn('Node.js version >= ' + minMajor + '.x required');
            process.exit(1);
        }
    }
}
nodejsVersionCheck();

const { main } = require('./lib/duktool/main');

let exitCode = 0;
main().then((res) => {
    logDebug('done');
}, (e) => {
    logWarn(e.stack || e);
    exitCode = 1;
}).finally(() => {
    require('./lib/util/fs').deleteTempDirectories();
}).then(() => {
    if (typeof process === 'object' && process !== null && typeof process.exit === 'function') {
        process.exit(exitCode);
    } else if (exitCode !== 0) {
        throw new TypeError('exit code ' + exitCode);
    }
});

// Duktape Promise polyfill.
if (typeof Promise === 'function' && typeof Promise.runQueue === 'function') {
    Promise.runQueue();
}
