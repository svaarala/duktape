'use strict';

const { readFileUtf8 } = require('../util/fs');

// Parse Duktape version from duktape.h.in.
function getDukVersion(filename) {
    var data = readFileUtf8(filename);
    var m = /^#define\s+DUK_VERSION\s+(.*?)L?\s*$/m.exec(data);
    if (m) {
        var dukVersion = Math.floor(Number(m[1]));
        var dukMajor = Math.floor(dukVersion / 10000)
        var dukMinor = Math.floor((dukVersion % 10000) / 100)
        var dukPatch = Math.floor(dukVersion % 100)
        var dukVersionFormatted = dukMajor + '.' + dukMinor + '.' + dukPatch;
        return { dukVersion, dukMajor, dukMinor, dukPatch, dukVersionFormatted };
    }
    throw new TypeError('cannot figure out Duktape version from ' + filename);
}
exports.getDukVersion = getDukVersion;
