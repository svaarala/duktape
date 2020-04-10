'use strict';

const { assert } = require('../util/assert');
const { pathJoin, fileExists } = require('../util/fs');

// Sanity check that we have a valid repo directory.
function validateRepoDirectory(dir) {
    assert(fileExists(pathJoin(dir, 'AUTHORS.rst')));
    assert(fileExists(pathJoin(dir, 'README.md')));
    assert(fileExists(pathJoin(dir, 'RELEASES.rst')));
    assert(fileExists(pathJoin(dir, 'src-input', 'duk_bi_object.c')));
    assert(fileExists(pathJoin(dir, 'src-tools', 'lib', 'builtins', 'gen_builtins.js')));
}
exports.validateRepoDirectory = validateRepoDirectory;
