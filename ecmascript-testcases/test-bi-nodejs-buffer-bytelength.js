/*
 *  Buffer.byteLength()
 */

/*@include util-nodejs-buffer.js@*/

/*===
byteLength test
0 undefined 0
0 utf8 0
0 ascii 0
0 dummy 0
3 undefined 3
3 utf8 3
3 ascii 3
3 dummy 3
7 undefined 9
7 utf8 9
7 ascii 9
7 dummy 9
0
16
123456
10
13
15
3
4
===*/

function byteLengthTest() {
    var b1 = new Buffer(0);
    var b2 = new Buffer(16);
    var b3 = new Buffer(123456);

    b1.fill(0x11);
    b2.fill(0xff);
    b3.fill(0x33);

    // The intended use of byteLength() is to indicate for an input string/encoding
    // pair how long a Buffer is needed.  For Duktape this is just the string's
    // internal byte length because we just use the string internal representation
    // and ignore the encoding.
    //
    // Duktape and Node.js differ on the 'ascii' encoding of 'foo\u1234bar'.
    // Node.js would encode it into 7 bytes, Duktape into 9 (CESU-8 internal
    // length).

    [ '', 'foo', 'foo\u1234bar' ].forEach(function (v) {
        [ undefined, 'utf8', 'ascii', 'dummy' ].forEach(function (encoding) {
            print(v.length, encoding, Buffer.byteLength(v, encoding));
        });
    });

    // Behavior for non-string inputs is not defined but there seems to be a ToString()
    // coercion for the input.

    // Avoid testing for Buffer.prototype: causes an assertion failure with Node.js
    // v0.12.1.  The cause seems to be string-coercion of Buffer.prototype:
    //
    // > String(Buffer.prototype)
    // node: ../src/node_buffer.cc:262: void node::Buffer::StringSlice(const v8::FunctionCallbackInfo<v8::Value>&) [with node::encoding encoding = (node::encoding)1u]: Assertion `obj_data != __null' failed.
    // Aborted (core dumped)

    // Buffers b1 and b3 are ASCII compatible.  Their byteLength() will match
    // the input byte length.  For buffer b2 (16 bytes) Node.js returns 48,
    // presumably because the string is invalid UTF-8 and coerces to 16 U+FFFD
    // characters, which are then UTF-8 encoded into 48 bytes.
    //
    // Duktape returns 16 for buffer b2 too.

    [ b1, b2, b3,
      { valueOf: function () { return 'dummydummy'; }, toString: function () { return 'dummydummy'; } },  // 10 bytes
      { valueOf: function () { return 'dummydummy\u1234'; }, toString: function () { return 'dummydummy\u1234' ; } },  // 13 bytes in UTF-8
      {},  // "[object Object]" -> 15 bytes
      123, // -> 3 bytes
      null // -> 4 bytes
    ].forEach(function (v) {
        print(Buffer.byteLength(v));
    });
}

try {
    print('byteLength test');
    byteLengthTest();
} catch (e) {
    print(e.stack || e);
}
