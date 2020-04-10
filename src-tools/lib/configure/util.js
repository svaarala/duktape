'use strict';

const { readFileUtf8, writeFileUtf8, pathJoin, copyFile } = require('../util/fs');
const { stripLastNewline } = require('../util/string_util');

function copyFiles(fileList, srcDir, dstDir) {
    for (let fn of fileList.sort()) {
        let srcFn = pathJoin(srcDir, fn);
        let dstFn = pathJoin(dstDir, fn);
        copyFile(srcFn, dstFn);
    }
}
exports.copyFiles = copyFiles;

function cBlockQuoteFile(fn) {
    var data = stripLastNewline(readFileUtf8(fn));
    var lines = data.split('\n').map(function (line) {
        return ' *  ' + line.replace(/[^\x20-\x7e]/g, function (c) { return '\\u' + ('0000' + c.charCodeAt(0).toString(16)).substr(-4); });
    });
    return '/*\n' + lines.join('\n') + '\n */';
}
exports.cBlockQuoteFile = cBlockQuoteFile;

function copyAndCQuote(srcFn, dstFn) {
    var data = cBlockQuoteFile(srcFn);
    writeFileUtf8(dstFn, data);
}
exports.copyAndCQuote = copyAndCQuote;

function copyFileUtf8AtSignReplace(srcFn, dstFn, replacements) {
    var res = readFileUtf8(srcFn).replace(/@(\w+)@/g, (m, a) => {
        return replacements[a] || '@' + a + '@';
    });
    writeFileUtf8(dstFn, res);
}
exports.copyFileUtf8AtSignReplace = copyFileUtf8AtSignReplace;
