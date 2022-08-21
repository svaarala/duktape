'use strict';

const { pathJoin, dirname, basename, mkdir } = require('../../util/fs');
const { asyncExecStdoutUtf8 } = require('../../util/exec');
const { configureSources } = require('../../configure/configure_sources');
const { getNowMillis } = require('../../util/time');
const { assert } = require('../../util/assert');
const { logDebug, logInfo } = require('../../util/logging');

// Compute cache key for Duktape binary and library caches.
function computeCacheKey({ forcedOptions }) {
    let cacheKey = JSON.stringify(forcedOptions);
    return cacheKey;
}

function prepareDuktape({ repoDirectory, outputDirectory, tempDirectory, forcedOptions = {}, fixupLines = [] }) {
    var sourceDirectory = pathJoin(repoDirectory, 'src-input');
    var configDirectory = pathJoin(repoDirectory, 'config');
    var licenseFile = pathJoin(repoDirectory, 'LICENSE.txt');
    var authorsFile = pathJoin(repoDirectory, 'AUTHORS.rst');
    var unicodeDataFile = pathJoin(sourceDirectory, 'UnicodeData.txt');
    var specialCasingFile = pathJoin(sourceDirectory, 'SpecialCasing.txt');
    var userBuiltinFiles = void 0;

    return configureSources({
        sourceDirectory,
        outputDirectory,
        configDirectory,
        tempDirectory,
        licenseFile,
        authorsFile,
        unicodeDataFile,
        specialCasingFile,
        dukDistMetaFile: void 0,
        gitCommit: void 0,
        gitDescribe: void 0,
        gitBranch: void 0,
        platform: void 0,
        architecture: void 0,
        compiler: void 0,
        forcedOptions,
        fixupLines,
        userBuiltinFiles,
        romAutoLightFunc: false,
        lineDirectives: false,
        c99TypesOnly: false,
        dll: false,
        romSupport: true,
        emitConfigSanityCheck: true,
        sanityStrict: true,
        useCppWarning: true,
        omitRemovedConfigOptions: true,
        omitDeprecatedConfigOptions: true,
        omitUnusedConfigOptions: true
    });
}

async function prepareDuktapeCached({ repoDirectory, outputDirectory, tempDirectory, forcedOptions = {}, fixupLines = [], testRunState }) {
    var cacheKey = computeCacheKey({ forcedOptions });
    var dukPreparePromise = testRunState.dukPrepareCache[cacheKey];
    if (dukPreparePromise) {
        logDebug('use cached duk prepare, key:', cacheKey, '->', await dukPreparePromise);
        return await dukPreparePromise;
    }
    logDebug('uncached duk prepare, key:', cacheKey);
    dukPreparePromise = Promise.resolve().then(() => {
        let tempCount = testRunState.dukPrepareCount++;
        let prepOut = pathJoin(tempDirectory, 'prep-' + tempCount);
        let prepTemp = pathJoin(tempDirectory, 'prep-temp-' + tempCount);
        mkdir(prepTemp);
        let prepareResult = prepareDuktape({
            repoDirectory,
            outputDirectory: prepOut,
            tempDirectory: prepTemp,
            forcedOptions
        });
        return { prepareResult, prepareOutputDirectory: prepOut };
    });

    testRunState.dukPrepareCache[cacheKey] = dukPreparePromise;
    return await dukPreparePromise;
}

async function compileDuktapeCommand({ repoDirectory, prepDirectory, tempDirectory, outputFilename }) {
    var extraPrintAlert = true;
    var extraModuleDuktape = true;
    var extraLogging = true;
    var compileStdout, compileStderr;
    var compileStartTime;
    var compileEndTime;
    var cmd = [];

    cmd.push('gcc');
    cmd.push('-o' + outputFilename);
    cmd.push('-O2');
    cmd.push('-std=c99');
    cmd.push('-Wall');
    cmd.push('-Wextra');
    cmd.push('-I' + prepDirectory);
    cmd.push('-I' + pathJoin(repoDirectory, 'examples', 'cmdline'));
    if (extraPrintAlert) {
        cmd.push('-I' + pathJoin(repoDirectory, 'extras', 'print-alert'));
    }
    if (extraModuleDuktape) {
        cmd.push('-I' + pathJoin(repoDirectory, 'extras', 'module-duktape'));
    }
    if (extraLogging) {
        cmd.push('-I' + pathJoin(repoDirectory, 'extras', 'logging'));
    }
    if (extraPrintAlert) {
        cmd.push('-DDUK_CMDLINE_PRINTALERT_SUPPORT');
    }
    if (extraModuleDuktape) {
        cmd.push('-DDUK_CMDLINE_MODULE_SUPPORT');
    }
    if (extraLogging) {
        cmd.push('-DDUK_CMDLINE_LOGGING_SUPPORT');
    }
    if (extraPrintAlert) {
        cmd.push(pathJoin(repoDirectory, 'extras', 'print-alert', 'duk_print_alert.c'));
    }
    if (extraModuleDuktape) {
        cmd.push(pathJoin(repoDirectory, 'extras', 'module-duktape', 'duk_module_duktape.c'));
    }
    if (extraLogging) {
        cmd.push(pathJoin(repoDirectory, 'extras', 'logging', 'duk_logging.c'));
    }
    cmd.push(pathJoin(repoDirectory, 'examples', 'cmdline', 'duk_cmdline.c'));
    cmd.push(pathJoin(prepDirectory, 'duktape.c'));

    cmd.push('-lm');
    cmd.push('-lpthread');
    logDebug(cmd);

    compileStartTime = getNowMillis();
    try {
        ({ stdout: compileStdout, stderr: compileStderr } = await asyncExecStdoutUtf8(cmd, {}));
        compileEndTime = getNowMillis();
    } catch (e) {
        compileEndTime = getNowMillis();
        logInfo(e);
    }

    return { dukCommandFilename: outputFilename, prepDirectory };
}

async function compileDuktapeLibrary({ repoDirectory, prepDirectory, tempDirectory, outputFilename }) {
    var compileStdout;
    var compileStartTime;
    var compileEndTime;
    var cmd = [];

    cmd.push('gcc');
    cmd.push('-c');
    cmd.push('-o' + outputFilename);
    cmd.push('-O2');
    cmd.push('-std=c99');
    cmd.push('-Wall');
    cmd.push('-Wextra');
    cmd.push('-I' + prepDirectory);
    cmd.push(pathJoin(prepDirectory, 'duktape.c'));
    cmd.push('-lm');
    cmd.push('-lpthread');
    logDebug(cmd);

    compileStartTime = getNowMillis();
    try {
        ({ stdout: compileStdout } = await asyncExecStdoutUtf8(cmd, {}));
        compileEndTime = getNowMillis();
    } catch (e) {
        compileEndTime = getNowMillis();
        logInfo(e);
    }

    return { dukLibraryFilename: outputFilename, prepDirectory };
}

async function compileCTestcase({ preparedFilename, dukLibraryFilename, prepDirectory, tempDirectory }) {
    assert(preparedFilename);
    assert(tempDirectory);

    var exeFilename = pathJoin(tempDirectory, 'testbinary');
    var cmd;
    var stdout, stderr;
    var startTime;
    var endTime;
    var exeFilename = pathJoin(tempDirectory, 'testbinary');
    var cmd;

    cmd = [];
    cmd.push('gcc');
    cmd.push('-o', exeFilename);
    cmd.push('-O2');
    cmd.push('-std=c99');
    cmd.push('-Wall');
    cmd.push('-Wextra');
    cmd.push('-I' + prepDirectory);
    if (dukLibraryFilename) {
        cmd.push('-L' + dirname(dukLibraryFilename));
    } else {
        cmd.push(pathJoin(prepDirectory, 'duktape.c'));
    }
    cmd.push(preparedFilename);
    if (dukLibraryFilename) {
        cmd.push(dukLibraryFilename);
    }
    cmd.push('-lm');
    cmd.push('-lpthread');
    logDebug(cmd);

    startTime = getNowMillis();
    try {
        ({ stdout, stderr } = await asyncExecStdoutUtf8(cmd, {}));
        endTime = getNowMillis();
    } catch (e) {
        endTime = getNowMillis();
        logInfo(e);
    }

    return { cExeFilename: exeFilename, stdout, startTime, endTime };
}

async function compileDukCommandCached({ repoDirectory, tempDirectory, testcaseMetadata, testRunState }) {
    var forcedOptions = testcaseMetadata.duktape_config || {};
    var cacheKey = computeCacheKey({ forcedOptions });

    var dukCommandPromise = testRunState.dukCommandCache[cacheKey];

    if (dukCommandPromise) {
        logDebug('use cached duk command, key:', cacheKey, '->', await dukCommandPromise);
        return await dukCommandPromise;
    }
    logDebug('uncached duk command, key:', cacheKey);

    var { prepareOutputDirectory } = await prepareDuktapeCached({
        repoDirectory,
        tempDirectory,
        forcedOptions,
        testRunState
    });

    dukCommandPromise = Promise.resolve().then(() => {
        let tempCount = testRunState.dukCommandCount++;
        let dukTemp = pathJoin(tempDirectory, 'dukcmd-temp-' + tempCount);
        let dukOut = pathJoin(tempDirectory, 'dukcmd-' + tempCount);
        mkdir(dukTemp);
        return compileDuktapeCommand({
            repoDirectory,
            prepDirectory: prepareOutputDirectory,
            tempDirectory: dukTemp,
            outputFilename: dukOut
        });
    });
    testRunState.dukCommandCache[cacheKey] = dukCommandPromise;

    return await dukCommandPromise;
}

async function compileDukLibraryCached({ repoDirectory, tempDirectory, testcaseMetadata, testRunState }) {
    var forcedOptions = testcaseMetadata.duktape_config || {};
    var cacheKey = computeCacheKey({ forcedOptions });

    var dukLibraryPromise = testRunState.dukLibraryCache[cacheKey];
    if (dukLibraryPromise) {
        logDebug('use cached duk library, key:', cacheKey, '->', await dukLibraryPromise);
        return await dukLibraryPromise;
    }
    logDebug('uncached duk library, key:', cacheKey);

    var { prepareOutputDirectory } = await prepareDuktapeCached({
        repoDirectory,
        tempDirectory,
        forcedOptions,
        testRunState
    });

    dukLibraryPromise = Promise.resolve().then(() => {
        let tempCount = testRunState.dukLibraryCount++;
        let dukTemp = pathJoin(tempDirectory, 'duklib-temp-' + tempCount);
        let dukOut = pathJoin(tempDirectory, 'duklib-' + tempCount + '.o');
        mkdir(dukTemp);
        return compileDuktapeLibrary({
            repoDirectory,
            prepDirectory: prepareOutputDirectory,
            tempDirectory: dukTemp,
            outputFilename: dukOut
        });
    });
    testRunState.dukLibraryCache[cacheKey] = dukLibraryPromise;

    return await dukLibraryPromise;
}

exports.compileDukCommandCached = compileDukCommandCached;
exports.compileDukLibraryCached = compileDukLibraryCached;
exports.compileCTestcase = compileCTestcase;
