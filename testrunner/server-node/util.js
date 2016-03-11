/*
 *  Misc utils.
 */

var fs = require('fs');
var crypto = require('crypto');

function sha1sum(x) {
    return crypto.createHash('sha1').update(x).digest('hex');
}

function sha1sumFile(x) {
    return sha1sum(fs.readFileSync(x));
}

function assert(x) {
    if (x) { return x; }
    throw new Error('assertion failed');
}

// For 'svaarala/duktape' returns 'duktape'.
function plainRepoName(fullRepoName) {
    t = /.*?\/(.*)$/.exec(fullRepoName);
    if (!t || !t[1]) {
        throw new Error('cannot get plain repo name from "' + fullRepoName + '"');
    }
    return t[1];
}

exports.sha1sum = sha1sum;
exports.sha1sumFile = sha1sumFile;
exports.assert = assert;
exports.plainRepoName = plainRepoName;
