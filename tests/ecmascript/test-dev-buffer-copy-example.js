/*
 *  Making a copy of a plain buffer with Ecmascript code.
 */

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
object [object ArrayBuffer] true ABCD
object [object ArrayBuffer] true ABCD
false
buf: ABCD
copy: XBCD
object [object ArrayBuffer] false ABCD
false
===*/

function bufferCopyTest() {
    // Create a plain buffer.
    var buf = Duktape.dec('hex', '41424344');  // ABCD
    print(typeof buf, buf, isPlainBuffer(buf), bufferToString(buf));

    // Plain buffer mimics ArrayBuffer so use .slice() to create a copy.
    // When called with a plain buffer the slice will also be a plain
    // buffer.
    //
    // A Duktape specific alternative would be:
    // var copy = ArrayBuffer.createPlain(buf);

    var copy = buf.slice();
    print(typeof copy, copy, isPlainBuffer(copy), bufferToString(buf));
    print(copy === buf);

    // Demonstrate independence.
    copy[0] = ('X').charCodeAt(0);
    print('buf:', bufferToString(buf));
    print('copy:', bufferToString(copy));

    // If argument to .slice() is Object coerced, the result is also a
    // full ArrayBuffer object.
    var copy = Object(buf).slice();
    print(typeof copy, copy, isPlainBuffer(copy), bufferToString(buf));
    print(copy === buf);
}

try {
    bufferCopyTest();
} catch (e) {
    print(e.stack || e);
}
