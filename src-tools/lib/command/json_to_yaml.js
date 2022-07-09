'use strict';

const { readFileUtf8, writeFileUtf8 } = require('../util/fs');
const { stringify: stringifyYaml } = require('../util/yaml');
const { assert } = require('../util/assert');

function jsonToYaml(filenameIn, filenameOut) {
    assert(filenameIn, 'input filename required');
    assert(filenameOut, 'output filename required');
    var doc = JSON.parse(readFileUtf8(filenameIn));
    var res = stringifyYaml(doc);
    writeFileUtf8(filenameOut, res);
}
exports.jsonToYaml = jsonToYaml;
