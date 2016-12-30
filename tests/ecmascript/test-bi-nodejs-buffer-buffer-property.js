/*
 *  Node.js v6.9.1 buffers are Uint8Array instances and have a .buffer property.
 */

/*===
true
false
[object ArrayBuffer]
===*/

function test() {
    var buf = new Buffer('abcdefgh');

    // Property exists.
    print('buffer' in buf);

    // In ES2015 this is false (also in Node.js v6.9.1) because the .buffer
    // property is an inherited accessor.
    print(Object.getOwnPropertyDescriptor(buf, 'buffer') != null);

    print(Object.prototype.toString.call(buf.buffer));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
