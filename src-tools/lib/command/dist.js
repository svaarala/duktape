'use strict';

const { distSources } = require('../dist/dist_sources');
const { createBareObject } = require('../util/bare');
const { createTempDir } = require('../util/fs');
const { assert } = require('../util/assert');

const distCommandSpec = createBareObject({
    description: 'Create a Duktape source distributable',
    options: createBareObject({
        'repo-directory': { type: 'path', default: void 0, description: 'Duktape repo directory root' },
        'output-directory': { short: 'o', type: 'path', default: void 0, description: 'Directory for output files (created automatically, must not exist)' },
        'temp-directory': { type: 'path', default: void 0, description: 'Force a specific temp directory (default is to create automatically)' },
        'validate-git': { type: 'boolean', default: false, value: true, description: 'Validate that distributed files are tracked by git and have no diff' }
    })
});
exports.distCommandSpec = distCommandSpec;

function distCommand(cmdline, autoDuktapeRoot) {
    var opts = cmdline.commandOpts;

    var repoDirectory = assert(opts['repo-directory'] || autoDuktapeRoot, '--repo-directory (or repo autodetect) needed');
    var outputDirectory = assert(opts['output-directory'], '--output-directory needed');
    var tempDirectory = assert(opts['temp-directory'] || createTempDir({}));

    distSources({
        repoDirectory,
        outputDirectory,
        tempDirectory,
        validateGit: opts['validate-git']
    });
}
exports.distCommand = distCommand;
