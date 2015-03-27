/*
 *  buf.fill() with a string argument.
 */

/*@include util-nodejs-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/* Custom because of some Node.js clamping differences. */

/*===
fill string test
16 bytes: 11111141626261416262111111111111
16 bytes: 11111141414141414141111111111111
16 bytes: 111111c39eecabbec39eec1111111111
16 bytes: 111111eda080eda080eda01111111111
16 bytes: 11111111111111111111111111111111
16 bytes: 11111111111111111111111111111111
===*/

function fillStringTest() {
    var b = new Buffer(16);

    // Fill value can also be a string.  It is repeated to fill the
    // buffer ('AbbaAbbaAbba'...).
    b.fill(0x11);
    b.fill('Abba', 3, 10);  // 'A' = 0x41 gets used
    printNodejsBuffer(b);

    // Single character string case; internally this is also a memset
    // so cover it explicitly.

    b.fill(0x11);
    b.fill('A', 3, 10);  // 'A' = 0x41 gets used
    printNodejsBuffer(b);

    // The fill pattern is actually quite interesting: here the string is
    // U+00DE U+CAFE.  Node.js uses the UTF-8 encoding of the string to
    // fill the buffer.  Here U+00DE encodes to C39E and U+CAFE to
    // ECABBE, so the fill pattern is C3 9E EC AB BE.  The fill length
    // is 8 bytes so what gets written in: C3 9E EC AB BE C3 9E EC.
    // A partial last codepoint may be written.
    //
    // For Duktape this fill pattern is mostly easy because we can just
    // use the memory representation directly.  It will only differ for
    // surrogate pairs.
    b.fill(0x11);
    b.fill('\u00de\ucafe', 3, 11);
    printNodejsBuffer(b);

    // Here's the surrogate pair case: U+D800 CESU-8.
    //
    // Node.js fill pattern is EF BF BD, which UTF-8 decodes to U+FFFD
    // (replacement character).
    //
    // For Duktape it is ED A0 80, which CESU-8 decodes to U+D800, i.e.
    // preserves the character.
    b.fill(0x11);
    b.fill('\ud800', 3, 11);
    printNodejsBuffer(b);

    // Empty string: fill gets silently ignored and buffer is not changed.
    b.fill(0x11);
    b.fill('', 3, 11);
    printNodejsBuffer(b);

    // Empty string: fill gets ignored and buffer is not changed, but
    // offset/length are still validated (but because they're clamped,
    // Duktape doesn't throw here).
    b.fill(0x11);
    try {
        b.fill('', 3, 17);
    } catch (e) {
        print(e.name);
    }
    printNodejsBuffer(b);
}

try {
    print('fill string test');
    fillStringTest();
} catch (e) {
    print(e.stack || e);
}
