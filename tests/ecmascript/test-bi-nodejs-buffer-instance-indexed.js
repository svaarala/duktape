/*
 *  Indexed properties
 */

/*@include util-nodejs-buffer.js@*/

/*===
Node.js Buffer indexed property test
0 128
1 129
2 130
3 131
4 132
5 133
6 134
7 135
8 bytes: 8081828384858687
8 bytes: 0007fe000001aa0b
===*/

function nodejsBufferIndexedPropertyTest() {
    var b;
    var i, n;

    b = new Buffer(8);
    for (i = 0, n = b.length; i < n; i++) {
        b[i] = 0x180 + i;  // use values >= 0x80 so that signedness is tested
    }
    for (i = 0, n = b.length; i < n; i++) {
        print(i, b[i]);
    }

    printNodejsBuffer(b);

    // String/char assignment goes through an integer coercion, e.g.
    // "123" coerces to 123.  Assigning a single character is mostly
    // useless.

    b[0] = '0';    // --> 0x00
    b[1] = '7';    // --> 0x07
    b[2] = '254';  // --> 0xfe
    b[3] = 'x';    // 'x' doesn't coerce to codepoint but NaN, and then 0
    b[4] = false;  // --> 0x00
    b[5] = true;   // --> 0x01
    b[6] = { valueOf: function () { return 0xaa; } };
    b[7] = [ '11' ];  // --> coerces to "11" -> 0x0b!

    printNodejsBuffer(b);
}

try {
    print('Node.js Buffer indexed property test');
    nodejsBufferIndexedPropertyTest();
} catch (e) {
    print(e.stack || e);
}
