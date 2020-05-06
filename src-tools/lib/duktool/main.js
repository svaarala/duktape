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

// Command line parsing spec.

const commandSpec = {
    options: createBareObject({
    }),
    commands: createBareObject({
        ['configure']: configureCommandSpec,
        ['dist']: distCommandSpec,
        ['decode-bytecode']: decodeBytecodeCommandSpec,
        ['dump-bytecode']: dumpBytecodeCommandSpec,
        ['generate-releases-rst']: generateReleasesRstCommandSpec
    })
};

// Locate src-input, in a Duktape repository or a dist directory, based on
// module.filename and some guesswork.
function locateDuktapeRoot() {
    if (!module || !module.filename) {
        return;
    }
    var currDir = dirname(module.filename);
    for (let i = 0; i < 5; i++) {  // sanity max levels
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

function main() {
    const enableDebug = false;

    if (!enableDebug) {
        console.debug = function nop() {};
        console.trace = function nop() {};
    }

    var args = getArgs();
    console.debug('args: ' + JSON.stringify(args));
    var cmdline = parseCommandLine(args, commandSpec);
    console.debug('cmdline: ' + JSON.stringify(cmdline));

    // Command line --help handling.
    if (cmdline.help) {
        console.log(cmdline.help);
        return;
    }

    // Locate Duktape root (used as a default by some commands).
    var autoDuktapeRoot = locateDuktapeRoot();

    // Commands.
    var commandMap = createBareObject({
        // Main commands: configure and prepare sources, dist.
        ['configure']: () => {
            configureCommand(cmdline, autoDuktapeRoot);
        },
        ['dist']: () => {
            distCommand(cmdline, autoDuktapeRoot);
        },
        ['decode-bytecode']: () => {
            decodeBytecodeCommand(cmdline, autoDuktapeRoot);
        },
        ['dump-bytecode']: () => {
            dumpBytecodeCommand(cmdline, autoDuktapeRoot);
        },
        ['generate-releases-rst']: () => {
            generateReleasesRstCommand(cmdline, autoDuktapeRoot);
        }
    });

    // Command switch.
    var commandString = cmdline.command;
    if (!commandString) {
        console.log('Missing command, see --help');
    } else {
        var command = commandMap[commandString];
        if (command) {
            command();
        } else {
            console.log('UNKNOWN COMMAND: ' + commandString);
        }
    }
}
exports.main = main;
