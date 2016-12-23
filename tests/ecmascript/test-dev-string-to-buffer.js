/*
 *  There are several ways of converting a string into a buffer; some forms
 *  will use the bytes of the internal string representation as is while others
 *  will coerce logical codepoints into buffer bytes.
 */

/*@include util-buffer.js@*/

/*===
|ff666f6f|
{type:"Buffer",data:[195,191,102,111,111]}
|c3bf666f6f|
===*/

function test() {
    var s, b;

    // Uint8Array accepts an array-like argument but not a string.  An input
    // string can be converted to an array using Array.prototype.map().  The
    // buffer bytes will be uint8-converted codepoints, not 1:1 with the
    // internal string representation (and not UTF-8 either).

    s = '\xfffoo';
    b = new Uint8Array(Array.prototype.map.call(s, function (v) {
        return v.charCodeAt(0);
    })).buffer;  // ArrayBuffer
    print(Duktape.enc('jx', b));

    // Node.js Buffer constructor accepts a string argument.  In Duktape the
    // string is converted to a buffer using the bytes of the internal
    // representation as is (this differs from Node.js).  For standard strings
    // this means that the buffer gets a CESU-8 representation of the string
    // (same as UTF-8 except for the surrogate pair range).
    //
    // Note that this allows an internal string to be converted into a buffer
    // as is which is *not* a sandboxing issue like the reverse operation.

    s = '\xfffoo';
    b = new Buffer(s);
    print(Duktape.enc('jx', b));

    // In Duktape 2.x there's an explicit plain buffer constructor which also
    // accepts a string argument, and copies bytes 1:1 from the string internal
    // representation to the buffer.

    s = '\xfffoo';
    b = Uint8Array.allocPlain(s);
    print(Duktape.enc('jx', b));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
