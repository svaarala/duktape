'use strict';

const { miscPolyfills } = require('./misc_polyfill');
miscPolyfills();

const { getArgs } = require('../extbindings/args');
const { parseCommandLine } = require('../util/cmdline');
const { dirname, pathJoin, fileExists } = require('../util/fs');
const { createBareObject } = require('../util/bare');

const { configureCommand, configureCommandSpec } = require('../command/configure');
const { distCommand, distCommandSpec } = require('../command/dist');
const { decodeBytecodeCommand, decodeBytecodeCommandSpec } = require('../command/decode_bytecode');
const { dumpBytecodeCommand, dumpBytecodeCommandSpec } = require('../command/dump_bytecode');
const { generateReleasesRstCommand, generateReleasesRstCommandSpec } = require('../command/generate_releases_rst');
const { prepTestCommand, prepTestCommandSpec } = require('../command/prep_test');
const { runTestsCommand, runTestsCommandSpec } = require('../command/run_tests');
const { tokenizeCommand, tokenizeCommandSpec } = require('../command/tokenize');
const { disableConsoleDebug } = require('../util/console');
const { logInfo, logDebug } = require('../util/logging');

// Command line parsing spec.

const commandSpec = {
    options: createBareObject({}),
    commands: createBareObject({
        ['configure']: configureCommandSpec,
        ['dist']: distCommandSpec,
        ['decode-bytecode']: decodeBytecodeCommandSpec,
        ['dump-bytecode']: dumpBytecodeCommandSpec,
        ['generate-releases-rst']: generateReleasesRstCommandSpec,
        ['prep-test']: prepTestCommandSpec,
        ['run-tests']: runTestsCommandSpec,
        ['tokenize']: tokenizeCommandSpec
    })
};

// Locate src-input, in a Duktape repository or a dist directory, based on
// module.filename and some guesswork.
function locateDuktapeRoot() {
    if (!module || !module.filename) {
        return;
    }
    var currDir = dirname(module.filename);
    for (let i = 0; i < 5; i++) { // sanity max levels
        console.debug('locate duktape root, currDir:', currDir);
        if (currDir === '') {
            break;
        }
        if (fileExists(pathJoin(currDir, 'src-input', 'duktape.h.in')) &&
            fileExists(pathJoin(currDir, 'AUTHORS.rst')) &&
            fileExists(pathJoin(currDir, 'LICENSE.txt'))) {
            return currDir;
        }
        currDir = pathJoin(currDir, '..');
    }
}

async function main() {
    const enableDebug = false;

    if (!enableDebug) {
        disableConsoleDebug();
    }

    // Locate Duktape root (used as a default by some commands).
    var autoDuktapeRoot = locateDuktapeRoot();

    var args = getArgs();
    console.debug('args: ' + JSON.stringify(args));
    var cmdline = parseCommandLine(args, commandSpec);
    console.debug('cmdline: ' + JSON.stringify(cmdline));

    // Command line --help handling.
    if (cmdline.help) {
        logInfo(cmdline.help);
        return;
    }

    // Commands.
    var commandMap = createBareObject({
        // Main commands: configure and prepare sources, dist.
        ['configure']: () => {
            return configureCommand(cmdline, autoDuktapeRoot);
        },
        ['dist']: () => {
            return distCommand(cmdline, autoDuktapeRoot);
        },
        ['decode-bytecode']: () => {
            return decodeBytecodeCommand(cmdline, autoDuktapeRoot);
        },
        ['dump-bytecode']: () => {
            return dumpBytecodeCommand(cmdline, autoDuktapeRoot);
        },
        ['generate-releases-rst']: () => {
            return generateReleasesRstCommand(cmdline, autoDuktapeRoot);
        },
        ['prep-test']: () => {
            return prepTestCommand(cmdline, autoDuktapeRoot);
        },
        ['run-tests']: () => {
            return runTestsCommand(cmdline, autoDuktapeRoot);
        },
        ['tokenize']: () => {
            return tokenizeCommand(cmdline, autoDuktapeRoot);
        }
    });

    // Command switch.
    var commandString = cmdline.command;
    if (!commandString) {
        logInfo('Missing command, see --help');
    } else {
        var command = commandMap[commandString];
        if (command) {
            logDebug('command start');
            await command();
            logDebug('command end');
        } else {
            logInfo('UNKNOWN COMMAND: ' + commandString);
        }
    }
}
exports.main = main;
