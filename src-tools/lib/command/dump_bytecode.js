'use strict';

const { decodeBytecodeUint8Array } = require('../bytecode/decode');
const { dumpBytecode } = require('../bytecode/dump');
const { pathJoin, readFile, readFileYaml, writeFileUtf8 } = require('../util/fs');
const { createBareObject } = require('../util/bare');

const dumpBytecodeCommandSpec = createBareObject({
    description: 'Dump a Duktape bytecode file into text form',
    options: createBareObject({
        'input': { type: 'path', short: 'i', default: void 0, required: true, description: 'input bytecode file' },
        'output': { type: 'path', short: 'o', default: void 0, description: 'output text dump file (dump to stdout if missing)' },
        'duk-opcodes': { type: 'path', default: void 0, description: 'duk_opcodes.yaml metadata file' },
        'jumps': { type: 'boolean', default: false, value: true, description: 'visualize jumps' }
    })
});
exports.dumpBytecodeCommandSpec = dumpBytecodeCommandSpec;

function dumpBytecodeCommand(cmdline, autoDuktapeRoot) {
    var opts = cmdline.commandOpts;
    var dukOpcodesFile = opts['duk-opcodes'];
    if (!dukOpcodesFile && autoDuktapeRoot) {
        dukOpcodesFile = pathJoin(autoDuktapeRoot, 'debugger', 'duk_opcodes.yaml');
    }
    var dukOpcodes = dukOpcodesFile ? readFileYaml(dukOpcodesFile) : void 0;
    var inputFile = opts['input'];
    var outputFile = opts['output'];
    var bcData = readFile(inputFile);

    var decoded = decodeBytecodeUint8Array(bcData);
    var dumped = dumpBytecode(decoded, { dukOpcodes, jumps: opts['jumps'] });
    if (outputFile) {
        writeFileUtf8(outputFile, dumped);
    } else {
        console.log(dumped);
    }
}
exports.dumpBytecodeCommand = dumpBytecodeCommand;
