/*
 *  String.fromBufferRaw() provided by the "duk" command.
 *
 *  NOTE: String.fromBufferRaw() is -not- part of the default built-ins.  It is
 *  provided by "duk" command to support testcases which need to deal with
 *  the internal string representation.
 */

/*@include util-buffer.js@*/
/*@include util-string.js@*/

/*---
custom: true
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
"\u07ad\ufffd\ufffd"
string "<U+07AD><U+FFFD><U+FFFD>"
|deadefbfbdefbfbd|
"\ufffd\ufffd\ufffd\ufffd"
string "<U+FFFD><U+FFFD><U+FFFD><U+FFFD>"
|efbfbdefbfbdefbfbdefbfbd|
"abcd"
string "abcd"
|61626364|
"\x00\x01\x02\ufffd\ufffd"
string "<U+0000><U+0001><U+0002><U+FFFD><U+FFFD>"
|000102efbfbdefbfbd|
"\ufffd\ufffd\ufffd\ufffdxV4\x12\ufffd\ufffd\ufffd\ufffd"
string "<U+FFFD><U+FFFD><U+FFFD><U+FFFD>xV4<U+0012><U+FFFD><U+FFFD><U+FFFD><U+FFFD>"
|efbfbdefbfbdefbfbdefbfbd78563412efbfbdefbfbdefbfbdefbfbd|
"\x00\x00\x00\x00\x00\x00\x00\x00\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd?\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd?\x00\x00\x00\x00\x00\x00\ufffd?333333\ufffd?\x00\x00\x00\x00\x00\x00\ufffd?ffffff\ufffd?\x00\x00\x00\x00\x00\x00\ufffd?\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd\ufffd?\x00\x00\x00\x00\x00@\ufffd@"
string "<U+0000><U+0000><U+0000><U+0000><U+0000><U+0000><U+0000><U+0000><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD>?<U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD>?<U+0000><U+0000><U+0000><U+0000><U+0000><U+0000><U+FFFD>?333333<U+FFFD>?<U+0000><U+0000><U+0000><U+0000><U+0000><U+0000><U+FFFD>?ffffff<U+FFFD>?<U+0000><U+0000><U+0000><U+0000><U+0000><U+0000><U+FFFD>?<U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD><U+FFFD>?<U+0000><U+0000><U+0000><U+0000><U+0000>@<U+FFFD>@"
|0000000000000000efbfbdefbfbdefbfbdefbfbdefbfbdefbfbdefbfbd3fefbfbdefbfbdefbfbdefbfbdefbfbdefbfbdefbfbd3f000000000000efbfbd3f333333333333efbfbd3f000000000000efbfbd3f666666666666efbfbd3f000000000000efbfbd3fefbfbdefbfbdefbfbdefbfbdefbfbdefbfbdefbfbd3f000000000040efbfbd40|
"\x04\x00\x05\x00\x06\x00\x07\x00"
string "<U+0004><U+0000><U+0005><U+0000><U+0006><U+0000><U+0007><U+0000>"
|0400050006000700|
===*/

function test() {
    var arrayBuf = new ArrayBuffer(4);
    var u8 = new Uint8Array(arrayBuf);
    u8[0] = 0xfe;  // avoid 0xFF -> interpreted as a symbol
    u8[1] = 0xee;
    u8[2] = 0xdd;
    u8[3] = 0xcc;

    [
        // All of these are rejected
        void 0, null, true, false, 'foo', '123',
        -1, 0, 1, 4.4, 4.5, 4.6, 128,
        'foo\ucafe\ufacebar',
        [ 0xde, 0xad, 0xbe, 0xef, '123', '234', '1234' ],
        { length: 4, 0: 100, 1: 101, 2: 102, 3: 103, 4: 104 },
        { 0: 100, 1: 101 },

        // Plain buffers are accepted
        Duktape.dec('hex', 'deadbeef'),

        // Buffer objects are accepted, active byte slice is used
        // (not element slice)
        arrayBuf,
        new Buffer('abcd'),
        new Uint8Array([ 0, 1, 2, 0xfe, 0xff ]),
        new Int32Array([ -0x02020202, 0x12345678, 0xcafeface ]),
        new Float64Array([ 0.0, 0.1, 0.4, 0.5, 0.6, 1.0, 1.4, 1.5, 1.6, 1000 ]),
        new Uint16Array([ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ]).subarray(3, 7)
    ].forEach(function (v) {
        try {
            var p = String.fromBufferRaw(v);
            print(Duktape.enc('jx', p));
            print(typeof p, safeEscapeString(p));
            print(Duktape.enc('jx', stringToBuffer(p)));
        } catch (e) {
            print(e.name);
        }
    });
}

test();
