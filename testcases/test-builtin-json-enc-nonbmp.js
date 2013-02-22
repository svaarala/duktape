/*---
{
    "custom": true
}
---*/

/*===
123 34 98 117 102 102 101 114 34 58 124 102 52 56 102 98 102 98 102 124 44 34 115 116 114 105 110 103 34 58 34 1114111 34 125
===*/

/* Test encoding of non-BMP characters. */

function nonBmpTest() {
    var buf;
    var t;
    var i;
    var tmp = [];

    // 0x10ffff = 1114111
    buf = __duk__.dec('hex', 'f48fbfbf');   // U+0010FFFF
    t = JSON.stringify({ buffer: buf, string: String(buf) });
    for (i = 0; i < t.length; i++) {
        tmp.push(t.charCodeAt(i));
    }

    print(tmp.join(' '));

    // FIXME: test ASCII-only encoding where the codepoint becomes "\U0010FFFF"
    // FIXME: test 32-bit xutf-8 codepoints
}

try {
    nonBmpTest();
} catch (e) {
    print(e.name);
}
