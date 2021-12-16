/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
|eda080edb080|
"\U00010000"
|eda080edaf80|
"\ud800\udbc0"
|edb080edb080|
"\udc00\udc00"
|edb080edaf80|
"\udc00\udbc0"
===*/

function test() {
    var buf, str;

    // >>> u'\ud800 \udc00'.encode('utf-8')
    // '\xed\xa0\x80 \xed\xb0\x80'

    // Surrogate pair encoded in CESU-8.  This should be rejected by string intern
    // keepcheck, requires sanitization and another string table lookup.
    buf = Uint8Array.allocPlain([ 0xed, 0xa0, 0x80, 0xed, 0xb0, 0x80 ]);
    str = bufferToStringRaw(buf);
    print(Duktape.enc('jx', buf));
    print(Duktape.enc('jx', str));

    buf = Uint8Array.allocPlain([ 0xed, 0xa0, 0x80, 0xed, 0xaf, 0x80 ]);
    str = bufferToStringRaw(buf);
    print(Duktape.enc('jx', buf));
    print(Duktape.enc('jx', str));

    buf = Uint8Array.allocPlain([ 0xed, 0xb0, 0x80, 0xed, 0xb0, 0x80 ]);
    str = bufferToStringRaw(buf);
    print(Duktape.enc('jx', buf));
    print(Duktape.enc('jx', str));

    buf = Uint8Array.allocPlain([ 0xed, 0xb0, 0x80, 0xed, 0xaf, 0x80 ]);
    str = bufferToStringRaw(buf);
    print(Duktape.enc('jx', buf));
    print(Duktape.enc('jx', str));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
