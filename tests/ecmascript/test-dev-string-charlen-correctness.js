/*
 *  Ensure string character length calculation is correct even for invalid
 *  UTF-8 and various string lengths.
 *
 *  The character length calculation is important for performance and the
 *  algorithm has optimizations which deal with e.g. different string lengths
 *  using separate code paths.
 */

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
done
===*/

function testOne(blen) {
    var buf, str;
    var i;
    var clen;

    buf = createPlainBuffer(blen);
    for (i = 0; i < blen; i++) {
        buf[i] = Math.random() * 256;
        // With Duktape 2.x some initial bytes cause a duk_hstring to be
        // interpreted as a symbol, avoid these.
        if (buf[i] == 0xff) { buf[i] = 0xfe; }
        if ((buf[i] & 0xc0) == 0x80) { buf[i] = 0xfe; }
    }

    // Expected character length computed using ECMAScript:
    // all bytes outside of [0x80,0xbf] (UTF-8 continuation
    // bytes) contribute +1 to character length.
    clen = 0;
    for (i = 0; i < blen; i++) {
        if (buf[i] < 0x80 || buf[i] >= 0xc0) {
            clen++;
        }
    }

    str = bufferToStringRaw(buf);
    if (str.length != clen) {
        throw new Error('mismatch: ' + str.length + ' vs ' + clen);
    }
}

function test() {
    var i, j;
    var blen;

    // Strings up to 256 bytes, a few times each
    for (i = 0; i <= 256; i++) {
        for (j = 0; j < 100; j++) {
            testOne(i);
        }
    }

    // Random strings up to 64kB, favor shorter strings
    // Math.exp(0) - 1 = 0
    // Math.exp(11.1) - 1 ~= 66170
    for (i = 0; i < 1000; i++) {
        blen = Math.max(Math.min(Math.floor(Math.exp(Math.random() * 11.1) - 1), 65536), 0);
        testOne(blen);
    }
}

try {
    test();
    print('done');
} catch (e) {
    print(e.stack);
}
