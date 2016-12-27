/*
 *  Duktape custom: Uint8Array.allocPlain()
 */

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
TypeError: invalid args
TypeError: invalid args
TypeError: invalid args
TypeError: invalid args
|666f6f|
|313233|
||
||
|00|
|00000000|
|00000000|
|00000000|
|0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000|
|666f6fecabbeefab8e626172|
|deadbeef|
|ffeeddcc|
|61626364|
|000102feff|
|ff78ce|
|000000000001010101e8|
|04050607|
|deadbeef7bead2|
|64656667|
||
===*/

function test() {
    var arrayBuf = new ArrayBuffer(4);
    var u8 = new Uint8Array(arrayBuf);
    u8[0] = 0xff;
    u8[1] = 0xee;
    u8[2] = 0xdd;
    u8[3] = 0xcc;

    [
        // All of these are rejected
        void 0, null, true, false, 'foo', '123',

        // Numbers are duk_to_int() coerced, negative values clamped
        -1, 0, 1, 4.4, 4.5, 4.6, 128,

        // String data gets copied as is
        'foo\ucafe\ufacebar',

        // Plain buffers are copied as is (but a new copy is made)
        Duktape.dec('hex', 'deadbeef'),

        // Buffer objects in general are copied as is, with their .length
        // used as the result length and items (not bytes!) copied.  For
        // e.g. Uint32Array this means only low byte of each 32-bit entry
        // gets used.  ArrayBuffer doesn't have a standard .length field
        // (Duktape 1.x provided .length; Duktape 2.x does not) so the outcome
        // is an empty plain buffer.
        arrayBuf,
        new Buffer('abcd'),
        new Uint8Array([ 0, 1, 2, 0xfe, 0xff ]),
        new Int32Array([ -1, 0x12345678, 0xcafeface ]),
        new Float64Array([ 0.0, 0.1, 0.4, 0.5, 0.6, 1.0, 1.4, 1.5, 1.6, 1000 ]),
        new Uint16Array([ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ]).subarray(3, 7),

        // Plain arrays are also accepted; entries are ToUint32() coerced and
        // then 0xff masked.
        [ 0xde, 0xad, 0xbe, 0xef, '123', '234', '1234' ],

        // Arbitrary object with .length is accepted
        { length: 4, 0: 100, 1: 101, 2: 102, 3: 103, 4: 104 },

        // Object without .length yields a zero length result
        { 0: 100, 1: 101 }
    ].forEach(function (v) {
        try {
            print(Duktape.enc('jx', Uint8Array.allocPlain(v)));
        } catch (e) {
            print(e);
        }
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
