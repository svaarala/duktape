/*
 *  There's no single, convenient standard idiom for converting a buffer
 *  object into a string.  Illustrate a few techniques.
 */

/*@include util-buffer.js@*/

/*===
4 "\xffabc"
|c3bf616263|
4 "\ufffdabc"
|efbfbd616263|
4 "\ufffdabc"
|efbfbd616263|
4 "\ufffdabc"
|efbfbd616263|
4 "\xfeabc"
|fe616263|
===*/

function test() {
    var b, s;

    // Reasonably clean and standard, but not (currently) very efficient.
    // Individual bytes in the argument ArrayBuffer or typed array are
    // interpreted as 16-bit codepoints, so the conversion is not 1:1 from
    // buffer to internal extended UTF-8 format.  As a result one cannot
    // create for example "internal keys".

    var u8 = new Uint8Array(4);
    u8[0] = 0xff;
    u8[1] = 0x61;
    u8[2] = 0x62;
    u8[3] = 0x63;
    s = String.fromCharCode.apply(null, u8);
    print(s.length, Duktape.enc('jx', s));
    print(Duktape.enc('jx', stringToBuffer(s)));

    // Node.js Buffer .toString() coercion decodes a buffer as UTF-8, emitting
    // replacement characters for invalid sequences.  Duktape 2.x follows this
    // behavior so one cannot create for example "internal keys".  (Duktape 1.x
    // used the buffer as-is as the string internal representation which *did*
    // allow internal keys to be created.)

    b = new Buffer(4);
    b[0] = 0xff;
    b[1] = 0x61;
    b[2] = 0x62;
    b[3] = 0x63;
    s = String(b);
    print(s.length, Duktape.enc('jx', s));
    print(Duktape.enc('jx', stringToBuffer(s)));

    // Buffer.prototype.toString() can also be called for other buffer
    // types in Duktape (not in other engines necessarily).

    b = new Uint8Array([ 0xff, 0x61, 0x62, 0x63 ]);
    s = Buffer.prototype.toString.call(b);
    print(s.length, Duktape.enc('jx', s));
    print(Duktape.enc('jx', stringToBuffer(s)));

    // WHATWG Encoding API is supported in Duktape 2.x.

    dec = new TextDecoder();
    b = new Uint8Array([ 0xff, 0x61, 0x62, 0x63 ]);
    s = dec.decode(b);
    print(s.length, Duktape.enc('jx', s));
    print(Duktape.enc('jx', stringToBuffer(s)));

    // In Duktape 2.x there's no default ECMAScript built-in for doing a
    // 1:1 string conversion, but "duk" command fills in String.fromBufferRaw().
    // The active slice of any buffer or buffer object argumented is
    // interpreted as bytes (even for e.g. Uint32Array) and copied 1:1 into
    // the internal string representation.
    //
    // Note that specific initial bytes are reserved for symbols; e.g. if
    // initial byte is 0xff the string coercion will actually create a
    // symbol because symbols and strings share the same internal
    // representation.

    b = new Uint8Array([ 0xfe, 0x61, 0x62, 0x63 ]);
    s = String.fromBufferRaw(b);
    print(s.length, Duktape.enc('jx', s));
    print(Duktape.enc('jx', stringToBuffer(s)));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
