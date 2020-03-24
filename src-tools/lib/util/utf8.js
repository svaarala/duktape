'use strict';

// Compute character length for an arbitrary byte string (bstr).
// Must match src-input/duk_unicode_support:duk_unicode_unvalidated_utf8_length().
function unvalidatedUtf8Length(x) {
    if (typeof x !== 'string') {
        throw new TypeError('argument must be a string');
    }
    var clen = 0;
    for (let i = 0; i < x.length; i++) {
        let o = x.charCodeAt(i);
        if (o > 0xff) {
            throw new TypeError('input must consist of U+0000 to U+00FF only');
        }
        if (o < 0x80 || o >= 0xc0) {
            // 0x80...0xbf are continuation chars, not counted
            clen++;
        }
    }
    return clen;
}
exports.unvalidatedUtf8Length = unvalidatedUtf8Length;
