/*
 *  There's no single, convenient standard idiom for converting a buffer object
 *  into a string.  Illustrate a few techniques.
 */

/*@include util-buffer.js@*/

/*===
4 "\xffabc"
|c3bf616263|
4 "\xffabc"
|ff616263|
===*/

function test() {
    var b, s;

    // Reasonably clean and standard, but not (currently) very efficient.
    // Individual bytes in the argument ArrayBuffer or typed array are
    // interpreted as 16-bit codepoints, so the conversion is not 1:1 from
    // buffer to internal extended UTF-8 format.  As a result one cannot
    // create for example "internal keys".

    b = new ArrayBuffer(4);
    b[0] = 0xff;
    b[1] = 0x61;
    b[2] = 0x62;
    b[3] = 0x63;
    s = String.fromCharCode.apply(null, b);
    print(s.length, Duktape.enc('jx', s));
    print(Duktape.enc('jx', stringToBuffer(s)));

    // Non-standard, works in both Duktape 1.x and 2.x: Node.js Buffer
    // string coercion is 1:1 into the internal key representation.

    b = new Buffer(4);
    b[0] = 0xff;
    b[1] = 0x61;
    b[2] = 0x62;
    b[3] = 0x63;
    s = String(b);
    print(s.length, Duktape.enc('jx', s));
    print(Duktape.enc('jx', stringToBuffer(s)));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
