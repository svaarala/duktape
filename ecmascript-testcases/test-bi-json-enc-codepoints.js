/*===
codepoint test (no output)
===*/

/* Test that all codepoint escapes can be parsed. */

function codepointTest() {
    var i;
    var str;
    var t;
    var nybbles = "0123456789abcdef";

    for (i = 0; i < 65536; i++) {
        str = '"\\u' +
              nybbles.charAt((i >> 12) & 0x0f) +
              nybbles.charAt((i >> 8) & 0x0f) +
              nybbles.charAt((i >> 4) & 0x0f) +
              nybbles.charAt((i >> 0) & 0x0f) +
              '"';
        t = JSON.parse(str);
        if (typeof t !== 'string') {
            throw new Error('result not string, codepoint: ' + i);
        }
        if (t.length !== 1) {
            throw new Error('result string length not 1, codepoint: ' + i);
        }
        if (t.charCodeAt(0) !== i) {
            throw new Error('result codepoint incorrect, codepoint: ' + i);
        }
    }
}

print('codepoint test (no output)');

try {
    codepointTest();
} catch (e) {
    print(e.name);
}
