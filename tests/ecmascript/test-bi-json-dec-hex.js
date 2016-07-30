/*
 *  JX hex decode fast path coverage
 *
 *  Relies on JX hex encode fast path being functional.
 */

/*@include util-buffer.js@*/

/*===
done
===*/

function bufferEquals(b1, b2) {
    var i;
    if (b1.length != b2.length) { return false; }
    for (i = 0; i < b1.length; i++) {
        if (b1[i] != b2[i]) { return false; }
    }
    return true;
}

function test() {
    var i, j, len;
    var buf, tmp;

    for (len = 0; len < 64; len++) {
        buf = createPlainBuffer(len);
        for (i = 0; i < buf.length; i++) {
            buf[i] = 0x55 * i;
        }

        // Vary key by 1 char to ensure both aligned and unaligned output for
        // hex data.
        tmp = Duktape.enc('jx', { foo: buf });
        if (!bufferEquals(Duktape.dec('jx', tmp).foo, buf)) { throw new Error('decode error'); }
        tmp = Duktape.enc('jx', { foox: buf });
        if (!bufferEquals(Duktape.dec('jx', tmp).foox, buf)) { throw new Error('decode error'); }
    }

    // March all bytes through an 11 byte long buffer (2 x 4 bytes fast path, 3 leftover).
    for (i = 0; i < 256; i++) {
        buf = createPlainBuffer(11);
        for (j = 0; j < 11; j++) {
            buf[j] = i + j;
        }
        tmp = Duktape.enc('jx', { foo: buf });
        if (!bufferEquals(Duktape.dec('jx', tmp).foo, buf)) { throw new Error('decode error'); }
        tmp = Duktape.enc('jx', { foox: buf });
        if (!bufferEquals(Duktape.dec('jx', tmp).foox, buf)) { throw new Error('decode error'); }
    }

    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
