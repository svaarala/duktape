'use strict';

const { writeFileUtf8 } = require('../util/fs');
const { createBareObject } = require('../util/bare');
const { releasesYamlToRst } = require('../website/releases_rst');

const generateReleasesRstCommandSpec = createBareObject({
    description: 'Generate RELEASES.rst from releases metadata',
    options: createBareObject({
        'input': { type: 'path', short: 'i', default: void 0, required: true, description: 'input releases.yaml file' },
        'output': { type: 'path', short: 'o', default: void 0, description: 'output RELEASES.rst file (dump to stdout if missing)' }
    })
});
exports.generateReleasesRstCommandSpec = generateReleasesRstCommandSpec;

function generateReleasesRstCommand(cmdline, autoDuktapeRoot) {
    void autoDuktapeRoot;
    var opts = cmdline.commandOpts;
    var inputFile = opts['input'];
    var outputFile = opts['output'];
    var rstData = releasesYamlToRst(inputFile);
    if (outputFile) {
        writeFileUtf8(outputFile, rstData);
    } else {
        console.log(rstData);
    }
}
exports.generateReleasesRstCommand = generateReleasesRstCommand;
