'use strict';

const jsYaml = require('../extdeps/js-yaml');
const { makeObjectsBareRecursive } = require('./bare');

// Parse input as a YAML document.  All objects in the returned document are
// made bare (inherit from null) so that it's safe to look up fields without
// risking hitting inherited properties (like 'toString').  Arrays maintain
// inheritance because there's less risk of accidental inheritance.  This
// allows array .forEach() etc work naturally.
function parse(input) {
    var doc = jsYaml.safeLoad(input);
    doc = makeObjectsBareRecursive(doc);
    return doc;
}
exports.parse = parse;

function stringify(input) {
    return jsYaml.safeDump(input);
}
exports.stringify = stringify;
