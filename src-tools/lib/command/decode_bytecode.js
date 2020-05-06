'use strict';

const { decodeBytecodeUint8Array } = require('../bytecode/decode');
const { readFile, writeFileJson } = require('../util/fs');
const { createBareObject } = require('../util/bare');

const decodeBytecodeCommandSpec = createBareObject({
    description: 'Decode a Duktape bytecode file into a JSON form',
    options: createBareObject({
        'input': { type: 'path', short: 'i', default: void 0, required: true, description: 'input bytecode file' },
        'output': { type: 'path', short: 'o', default: void 0, required: true, description: 'output JSON dump file' },
        'duk-opcodes': { type: 'path', default: void 0, description: 'duk_opcodes.yaml metadata file' }
    })
});
exports.decodeBytecodeCommandSpec = decodeBytecodeCommandSpec;

function decodeBytecodeCommand(cmdline, autoDuktapeRoot) {
    void autoDuktapeRoot;
    var opts = cmdline.commandOpts;
    void opts;
    var inputFile = opts['input'];
    var outputFile = opts['output'];
    var bcData = readFile(inputFile);

    var res = decodeBytecodeUint8Array(bcData);
    writeFileJson(outputFile, res);
}
exports.decodeBytecodeCommand = decodeBytecodeCommand;
