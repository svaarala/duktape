'use strict';

const { readFileUtf8, writeFileUtf8 } = require('../util/fs');
const { parse: parseYaml } = require('../util/yaml');
const { assert } = require('../util/assert');

function yamlToJson(filenameIn, filenameOut) {
    assert(filenameIn, 'input filename required');
    assert(filenameOut, 'output filename required');
    var doc = parseYaml(readFileUtf8(filenameIn));
    var res = JSON.stringify(doc);
    writeFileUtf8(filenameOut, res);
}
exports.yamlToJson = yamlToJson;
