/*
 *  Duktape custom: Uint8Array.plainOf()
 */

/*---
{
    "custom": true
}
---*/

/*===
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
|deadbeef|
true
|ffeeddcc|
false
|61626364|
false
|000102feff|
false
|ffffffff78563412cefafeca|
false
|00000000000000009a9999999999b93f9a9999999999d93f000000000000e03f333333333333e33f000000000000f03f666666666666f63f000000000000f83f9a9999999999f93f0000000000408f40|
false
|0100020003000400050006000700080009000a00|
false
===*/

function test() {
    var arrayBuf = new ArrayBuffer(4);
    var u8Buf = new Uint8Array(arrayBuf);
    u8Buf[0] = 0xff;
    u8Buf[1] = 0xee;
    u8Buf[2] = 0xdd;
    u8Buf[3] = 0xcc;

    [
        // All of these are rejected
        void 0, null, true, false, 'foo', '123',
        -1, 0, 1, 4.4, 4.5, 4.6, 128,
        'foo\ucafe\ufacebar',
        [ 0xde, 0xad, 0xbe, 0xef, '123', '234', '1234' ],
        { length: 4, 0: 100, 1: 101, 2: 102, 3: 103, 4: 104 },
        { 0: 100, 1: 101 },

        // Plain buffers are returned as is
        Duktape.dec('hex', 'deadbeef'),

        // Plain buffer underlying a buffer object is returned as is
        arrayBuf,
        new Buffer('abcd'),
        new Uint8Array([ 0, 1, 2, 0xfe, 0xff ]),
        new Int32Array([ -1, 0x12345678, 0xcafeface ]),
        new Float64Array([ 0.0, 0.1, 0.4, 0.5, 0.6, 1.0, 1.4, 1.5, 1.6, 1000 ]),
        new Uint16Array([ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ]).subarray(3, 7)
    ].forEach(function (v) {
        try {
            var p = Uint8Array.plainOf(v);
            print(Duktape.enc('jx', p));
            print(p === v);
        } catch (e) {
            print(e.name);
        }
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
