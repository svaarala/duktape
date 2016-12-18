/*
 *  User code can access internal keys by constructing suitable
 *  property names e.g. through buffers.
 *
 *  To prevent this in sandboxing, user code must have no access to any
 *  buffer values nor the ability to create buffer values.
 */

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
date: 1970-01-01T00:02:03.456Z
using ArrayBuffer, date \xFFValue: 123456
using Duktape.dec, date \xFFValue: 123456
===*/

function test() {
    var dt = new Date(123456);  // has internal property \xFFValue
    var buf;
    var key;

    print('date:', dt.toISOString());

    // Using ArrayBuffer()
    buf = new ArrayBuffer(6);
    buf[0] = 0xff;
    buf[1] = 'V'.charCodeAt(0);
    buf[2] = 'a'.charCodeAt(0);
    buf[3] = 'l'.charCodeAt(0);
    buf[4] = 'u'.charCodeAt(0);
    buf[5] = 'e'.charCodeAt(0);
    key = bufferToString(buf);
    print('using ArrayBuffer, date \\xFFValue:', dt[key]);

    // Using Duktape.dec()
    key = bufferToString(Duktape.dec('hex', 'ff56616c7565'));  // \xFFValue
    print('using Duktape.dec, date \\xFFValue:', dt[key]);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
