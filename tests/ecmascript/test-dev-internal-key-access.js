/*
 *  User code can access internal keys by constructing suitable
 *  property names e.g. through buffers.
 *
 *  In Duktape 2.x standard built-ins apply an encoding when converting a
 *  buffer to a string which prevents internal keys from being created.
 *  However, C code can quite easily convert a buffer to a string without
 *  an encoding, so sandboxing environments must carefully avoid doing so
 *  in their C bindings.
 */

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
date: 1970-01-01T00:02:03.456Z
using ArrayBuffer, date \x82Value: 123456
using Duktape.dec, date \x82Value: 123456
===*/

function test() {
    var dt = new Date(123456);  // has internal property \x82Value
    var buf;
    var u8;
    var key;

    print('date:', dt.toISOString());

    // Using ArrayBuffer()
    buf = new ArrayBuffer(6);
    u8 = new Uint8Array(buf);
    u8[0] = 0x82;
    u8[1] = 'V'.charCodeAt(0);
    u8[2] = 'a'.charCodeAt(0);
    u8[3] = 'l'.charCodeAt(0);
    u8[4] = 'u'.charCodeAt(0);
    u8[5] = 'e'.charCodeAt(0);
    key = bufferToStringRaw(buf);  // currently uses String.fromBufferRaw(), custom binding provided by 'duk'
    print('using ArrayBuffer, date \\x82Value:', dt[key]);

    // Using Duktape.dec()
    key = bufferToStringRaw(Duktape.dec('hex', '8256616c7565'));  // \x82Value
    print('using Duktape.dec, date \\x82Value:', dt[key]);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
