/*
 *  User code can access internal keys by constructing suitable
 *  property names e.g. through buffers.
 *
 *  To prevent this in sandboxing, user code must have no access to any
 *  buffer values nor the ability to create buffer values.
 */

/*---
{
    "custom": true
}
---*/

/*===
date: 1970-01-01T00:02:03.456Z
using Duktape.Buffer, date \xFFValue: 123456
using Duktape.dec, date \xFFValue: 123456
===*/

function test() {
    var dt = new Date(123456);  // has internal property \xFFValue
    var buf;
    var key;

    print('date:', dt.toISOString());

    // Using Duktape.Buffer()
    buf = new Duktape.Buffer(1);
    buf[0] = 0xff;
    key = buf + 'Value';
    print('using Duktape.Buffer, date \\xFFValue:', dt[key]);

    // Using Duktape.dec()
    key = Duktape.dec('hex', 'ff56616c7565');  // \xFFValue
    print('using Duktape.dec, date \\xFFValue:', dt[key]);
}

try {
    test();
} catch (e) {
    print(e);
}
