/*
 *  Node.js v6.9.1 buffers are Uint8Array instances and have a .buffer property.
 */

/*===
true
true
[object ArrayBuffer]
===*/

function test() {
    var buf = new Buffer('abcdefgh');

    // Property exists.
    print('buffer' in buf);

    // In ES6 this should be false (also in Node.js v6.9.1) because the .buffer
    // property is an inherited accessor.  In the current Duktape implementation
    // it is a concrete property so this is true now.
    print(Object.getOwnPropertyDescriptor(buf, 'buffer') != null);

    print(Object.prototype.toString.call(buf.buffer));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
