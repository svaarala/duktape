'use strict';

// Stringify to one-line pure-ASCII JSON.
function jsonStringifyAscii(x) {
    return JSON.stringify(x).replace(/[\u0000-\u001f\u007f-\uffff]/g, (c) => {
        return '\\u' + ('0000' + c.charCodeAt(0).toString(16)).substr(-4);
    });
}
exports.jsonStringifyAscii = jsonStringifyAscii;
