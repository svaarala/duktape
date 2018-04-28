/*
 *  Making a copy of a plain buffer with ECMAScript code.
 */

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
object [object Uint8Array] true ABCD
object [object Uint8Array] true ABCD
false
buf: ABCD
copy: XBCD
===*/

function bufferCopyTest() {
    // Create a plain buffer.
    var buf = createPlainBuffer('ABCD');
    print(typeof buf, buf, isPlainBuffer(buf), bufferToStringRaw(buf));

    // Plain buffer mimics Uint8Array so it can be used as an initializer
    // for e.g. new Uint8Array().  The result will be an Uint8Array object
    // but can be converted to a plain buffer using Uint8Array.plainOf().
    //
    // A Duktape specific alternative which creates a plain buffer directly:
    // var copy = Uint8Array.createPlain(buf);

    var copy = Uint8Array.plainOf(new Uint8Array(buf));
    print(typeof copy, copy, isPlainBuffer(copy), bufferToStringRaw(buf));
    print(copy === buf);

    // Demonstrate independence.
    copy[0] = ('X').charCodeAt(0);
    print('buf:', bufferToStringRaw(buf));
    print('copy:', bufferToStringRaw(copy));
}

try {
    bufferCopyTest();
} catch (e) {
    print(e.stack || e);
}
