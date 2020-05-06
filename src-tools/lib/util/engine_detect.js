'use strict';

function isNodejs() {
    return typeof process === 'object' && process !== null && typeof process.cwd === 'function';
}
exports.isNodejs = isNodejs;

function isDuktape() {
    return typeof Duktape === 'object' && Duktape !== null;
}
exports.isDuktape = isDuktape;
