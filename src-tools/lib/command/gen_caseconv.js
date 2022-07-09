'use strict';

const { readFileUtf8, writeFile } = require('../util/fs');
const { createConversionMaps, removeConversionMapAscii, generateCaseconvTables } = require('../unicode/case_conversion');
const { parseUnicodeText, parseUnicodeXml } = require('../unicode/parser');

function generateCaseconv(unicodeDataFilename, specialCasingFilename) {
    var t = parseUnicodeText(readFileUtf8(unicodeDataFilename), readFileUtf8(specialCasingFilename));
    var convmap = createConversionMaps(t);
    var uc = convmap.uc;
    removeConversionMapAscii(convmap.uc);
    var be = generateCaseconvTables(uc);
    writeFile('/tmp/out.bin', be.getBytes());
}
exports.generateCaseconv = generateCaseconv;

function generateCaseconvXml(ucdAllFlatFilename) {
    var t = parseUnicodeXml(readFileUtf8(ucdAllFlatFilename))
    var convmap = createConversionMaps(t);
    var uc = convmap.uc;
    removeConversionMapAscii(convmap.uc);
    var be = generateCaseconvTables(uc);
    writeFile('/tmp/out.bin', be.getBytes());
}
exports.generateCaseconvXml = generateCaseconvXml;
