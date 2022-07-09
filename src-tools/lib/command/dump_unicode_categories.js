'use strict';

const { parseUnicodeText } = require('../unicode/parser');
const { extractCategories } = require('../unicode/categories');
const { codepointSequenceToRanges, rangesToPrettyRanges } = require('../unicode/util');
const { readFileUtf8, writeFileUtf8, writeFileJsonPretty } = require('../util/fs');
const { assert } = require('../util/assert');

function dumpUnicodeCategoriesCommand(cmdline, autoDuktapeRoot) {
    var unicodeDataFile = assert(cmdline.opts['unicode-data']);
    var specialCasingFile = assert(cmdline.opts['special-casing']);
    var outputJsonFile = assert(cmdline.opts['output-json']);
    var outputTextFile = assert(cmdline.opts['output-text']);
    void autoDuktapeRoot;

    var cpMap = parseUnicodeText(readFileUtf8(unicodeDataFile), readFileUtf8(specialCasingFile));
    var cats = extractCategories(cpMap);
    var text = [];
    text.push('Category dumps:');
    for (let cat of Object.getOwnPropertyNames(cats).sort()) {
        text.push('');
        text.push(cat);
        text = text.concat(rangesToPrettyRanges(codepointSequenceToRanges(cats[cat])).map((line) => '- ' + line));
    }
    writeFileJsonPretty(outputJsonFile, cats);
    writeFileUtf8(outputTextFile, text.join('\n') + '\n');
}
exports.dumpUnicodeCategoriesCommand = dumpUnicodeCategoriesCommand;
