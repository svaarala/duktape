/*
 *  Assigning a non-number to a buffer index
 */

/*@include util-nodejs-buffer.js@*/

/*===
8 bytes: 1111111111111111
8 bytes: 1111001111111111
8 bytes: 11117b1111111111
8 bytes: 1111641111111111
8 bytes: 1111c01111111111
8 bytes: 1111001111111111
8 bytes: 1111011111111111
8 bytes: 1111c91111111111
===*/

function nodejsBufferAssignNonNumberTest() {
    var b1 = new Buffer(8);
    b1.fill(0x11);
    printNodejsBuffer(b1);

    // 0x00 gets assigned
    b1[2] = 'X';
    printNodejsBuffer(b1);

    // 0x7b gets assigned, i.e. value gets ToNumber() coerced (or similar)
    b1[2] = '123';
    printNodejsBuffer(b1);

    // 0x64 gets assigned
    b1[2] = '1e2';
    printNodejsBuffer(b1);

    // 0xc0 gets assigned
    b1[2] = '192.9';
    printNodejsBuffer(b1);

    // false coerces to 0
    b1[2] = false;
    printNodejsBuffer(b1);

    // true coerces to 1
    b1[2] = true;
    printNodejsBuffer(b1);

    // valueOf coercion
    b1[2] = { valueOf: function () { return 0xc9; } };
    printNodejsBuffer(b1);
}

try {
    nodejsBufferAssignNonNumberTest();
} catch (e) {
    print(e.stack || e);
}
