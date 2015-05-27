/*
 *  Simple view testing.
 */

/*---
{
    "custom": true,
    "endianness": "little"
}
---*/

/*===
v1 0 1
v1 1 4294967295
v1 2 127
v1 3 52
v2 0 1
v2 1 -1
v2 2 127
v2 3 52
b1 0 17
b1 1 17
b1 2 17
b1 3 17
b1 4 17
b1 5 17
b1 6 17
b1 7 17
b1 8 17
b1 9 17
b1 10 17
b1 11 17
b1 12 17
b1 13 17
b1 14 17
b1 15 17
b1 0 17
b1 1 17
b1 2 17
b1 3 17
b1 4 17
b1 5 17
b1 6 17
b1 7 255
b1 8 1
b1 9 127
b1 10 128
b1 11 17
b1 12 17
b1 13 17
b1 14 17
b1 15 17
b1 0 255
b1 1 255
b1 2 255
b1 3 255
b1 4 1
b1 5 0
b1 6 0
b1 7 0
b1 8 127
b1 9 0
b1 10 0
b1 11 0
b1 12 128
b1 13 255
b1 14 255
b1 15 255
v1 0 0
v1 1 127
v1 2 0
v1 3 0
v2 0 4294967295
v2 1 1
v2 2 127
v2 3 4294967168
===*/

function test() {
    var b1 = new ArrayBuffer(16);
    var v1 = new Uint32Array(b1);
    var b2 = new ArrayBuffer(4);
    var v2 = new Int8Array(b2);
    var i;

    v2[0] = 1;
    v2[1] = 0xff;
    v2[2] = 0x7f;
    v2[3] = 0x1234;

    // Byte values in v2 get expanded to 32-bit values in v1.
    v1.set(v2);

    for (i = 0; i < v1.length; i++) { print('v1', i, v1[i]); }
    for (i = 0; i < v2.length; i++) { print('v2', i, v2[i]); }
//    print(Duktape.enc('jx', v1));
//    print(Duktape.enc('jx', v2));

    // NOTE: "Same underlying buffer" case cannot be handled as a memmove-like
    // situation because e.g. 4 bytes in the middle of a buffer may expand to
    // (up to) 32 bytes in the conversion.  That 32 bytes is both "before" and
    // "after" the source:
    //
    // |              ABCD              |   4 bytes, source
    // |aaaaaaaabbbbbbbbccccccccdddddddd|   32 bytes, target
    //
    // The only easy solution in this case is to make an actual copy.

    b1 = new ArrayBuffer(16);
    for (i = 0; i < b1.byteLength; i++) { b1[i] = 0x11; }
    for (i = 0; i < b1.byteLength; i++) { print('b1', i, b1[i]); }
    v1 = new Int8Array(b1, 7, 4);    // source: bytes [7,11[
    v2 = new Uint32Array(b1);        // target: bytes [0,16[
    v1[0] = -1;
    v1[1] = 1;
    v1[2] = 0x7f;
    v1[3] = -0x80;
    for (i = 0; i < b1.byteLength; i++) { print('b1', i, b1[i]); }
    v2.set(v1);
    for (i = 0; i < b1.byteLength; i++) { print('b1', i, b1[i]); }

    // b1:
    // 11111111 11111111 11111111 11111111     initial state
    // 11111111 111111ff 017f8011 11111111     after writing to v1
    // ffffffff 01000000 7f000000 80ffffff     after set()

    for (i = 0; i < v1.length; i++) { print('v1', i, v1[i]); }
    for (i = 0; i < v2.length; i++) { print('v2', i, v2[i]); }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
