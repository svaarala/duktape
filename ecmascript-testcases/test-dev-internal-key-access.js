/*
 *  User code can access internal keys by constructing suitable
 *  property names e.g. through buffers.
 *
 *  To prevent this in sandboxing, user code must have no access to any
 *  buffer values nor the ability to create buffer values.
 */

/*===
date: 1970-01-01T00:02:03.456Z
using Duktape.Buffer, date \xFFvalue: 123456
using Duktape.dec, date \xFFvalue: 123456
===*/

function test() {
    var dt = new Date(123456);  // has internal property \xFFvalue
    var buf;
    var key;

    print('date:', dt.toISOString());

    // Using Duktape.Buffer()
    buf = new Duktape.Buffer(1);
    buf[0] = 0xff;
    key = buf + 'value';
    print('using Duktape.Buffer, date \\xFFvalue:', dt[key]);

    // Using Duktape.dec()
    key = Duktape.dec('hex', 'ff76616c7565');  // \xFFvalue
    print('using Duktape.dec, date \\xFFvalue:', dt[key]);
}

try {
    test();
} catch (e) {
    print(e);
}
