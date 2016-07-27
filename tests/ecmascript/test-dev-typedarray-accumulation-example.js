/*
 *  Example of accumulating data to an ArrayBuffer reasonably efficiently
 *  without resorting to Duktape specific features.  Neither standard
 *  ArrayBuffer nor Node.js Buffer is resizable, so accumulation must use
 *  some kind of manual reallocation.
 *
 *  Duktape specific code can use a plain dynamic buffer to accumulate data;
 *  it can be resized on-the-fly without manual copying.
 */

/*===
resize from 64 to 128
resize from 128 to 256
resize from 256 to 512
resize from 512 to 1024
resize from 1024 to 2048
resize from 2048 to 4096
resize from 4096 to 8192
Final length: 4500 (wrote 4500)
===*/

// Accumulation buffer, [0,offset[ is filled, [offset,data.byteLength[ is
// not yet filled.
var data = new Uint8Array(64);
var offset = 0;

function received(buf) {
    // Incoming data ('buf') is an ArrayBuffer.  Use .byteLength throughout
    // because it's available for both ArrayBuffers and Uint8Arrays; for
    // standard ArrayBuffers a .length is not available.

    while (data.byteLength - offset < buf.byteLength) {
        // Not enough space, resize to make room.  Factor could be other
        // than 2, e.g. 1.5.
        print('resize from', data.byteLength, 'to', data.byteLength * 2);
        var newBuf = new Uint8Array(data.byteLength * 2);
        newBuf.set(data);  // copy old bytes
        data = newBuf;
    }

    data.set(new Uint8Array(buf), offset);
    offset += buf.byteLength;
}

function finalize() {
    // When accumulation is finished, final data can be extracted as follows:
    var finalArrayBuffer = data.buffer.slice(0, offset);
    return finalArrayBuffer;
}

function test() {
    var i, len, u8, res, wrote = 0;

    for (i = 0; i < 1000; i++) {
        len = i % 10;
        u8 = new Uint8Array(len);
        for (j = 0; j < len; j++) {
            u8[j] = 0x80 + j;
        }
        received(u8.buffer);
        wrote += u8.byteLength;
    }

    res = finalize();
    print('Final length: ' + res.byteLength + ' (wrote ' + wrote + ')');

    //print(Duktape.enc('jx', res));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
