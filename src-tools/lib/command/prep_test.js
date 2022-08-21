'use strict';

const { writeFileUtf8 } = require('../util/fs');
const { prepareTestcase } = require('../testing/duktape/prep_test');
const { createBareObject } = require('../util/bare');

const prepTestCommandSpec = createBareObject({
    description: 'Prepare a Duktape testcases for compilation/execution',
    options: createBareObject({
        'input': { type: 'path', short: 'i', default: void 0, required: true, description: 'input testcase file' },
        'output': { type: 'path', short: 'o', default: void 0, description: 'output prepared file' },
        'uglifyjs-bin': { type: 'path', default: void 0, description: 'Path to UglifyJS binary for minifying' }
    })
});
exports.prepTestCommandSpec = prepTestCommandSpec;

function prepTestCommand({ commandOpts }, autoDuktapeRoot) {
    var opts = commandOpts;
    var { preparedSource } = prepareTestcase({
        repoDirectory: autoDuktapeRoot,
        testcaseFilename: opts.input,
        uglifyJs2ExePath: commandOpts['uglifyjs-bin'],
        tryCatchWrapper: true,
        tryCatchRethrow: true
    });
    if (opts.output) {
        writeFileUtf8(opts.output, preparedSource);
    } else {
        console.log(preparedSource);
    }
}
exports.prepTestCommand = prepTestCommand;
