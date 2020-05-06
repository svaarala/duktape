'use strict';

function assert(x, message) {
    if (x) {
        return x;
    }
    throw new TypeError('assertion failed' + (message ? ': ' + message : ''));
}
exports.assert = assert;
