/*
 *  Test memory safety of decoding a JSON document clipped at all possible
 *  offsets, and any possible byte (or none) appended.
 *
 *  The JSON document should exercise all (or as many as possible) code paths
 *  of JSON parsing.  This is especially important after the change to remove
 *  an explicit end-of-input check and rely on a SyntaxError caused by (a
 *  guaranteed) NUL instead.
 *
 *  We don't care about correct parsing in this test -- except to test that
 *  the full document with no trailing bytes parses correctly.
 */

/*---
{
    "custom": true
}
---*/

/*===
input length: 195
input length: 195
input length: 195
input length: 627
input length: 627
input length: 627
input length: 6
6 -1 SyntaxError
input length: 6
6 -1 SyntaxError
input length: 6
6 -1 SyntaxError
input length: 26
input length: 116
28 124 TypeError
30 124 TypeError
32 124 TypeError
34 124 TypeError
===*/

function jsonClipTest(input, parser) {
    var i, j;
    var t, buf;

    print('input length: ' + input.length);

    for (i = 0; i <= input.length; i++) {
        for (j = -1; j < 256; j++) {
            try {
                t = input.substring(0, i);
                if (j >= 0) {
                    buf = Duktape.Buffer(1);
                    buf[0] = j;
                    t = t + buf;
                }
                if (parser === 'json') {
                    t = JSON.parse(t);
                } else if (parser === 'jx') {
                    t = Duktape.dec('jx', t);
                } else if (parser === 'jc') {
                    t = Duktape.dec('jc', t);
                } else {
                    throw new Error('invalid parser: ' + parser);
                }
            } catch (e) {
                // print error if original input doesn't parse (expected for some tests)
                if ((i == input.length && j == -1) || e.name !== 'SyntaxError') {
                    print(i, j, e.name);
                }
            }
        }
    }
}

function test() {
    var inp;
    var i, j;

    // Exercise:
    // - all types
    // - all escapes
    inp = '{ ' +
          '"types": [ null, true, false, 123, 123.456, 123e3, "", "val", { "foo": "bar" }, [ 1, 2, 3 ] ], ' +
          '"escapes": "foo\\t\\n\\r\\f\\b\\u1234\\ubeef\\u0000bar", ' +
          '"emptyobj": {}, ' +
          '"emptyarr": [], ' +
          '"dummy":"value" ' +
          '}';
    jsonClipTest(inp, 'json');
    jsonClipTest(inp, 'jx');
    jsonClipTest(inp, 'jc');

    // Exercise:
    // - All codepoints 0x00...0xff
    // - All UTF-8 encoding lengths

    for (i = 0; i < 256; i++) {
        inp += String.fromCharCode(i);
    }
    inp += '\u0123\ubeef';  // 2 and 3 byte
    inp = JSON.stringify(inp);
    jsonClipTest(inp, 'json');
    jsonClipTest(inp, 'jx');
    jsonClipTest(inp, 'jc');

    // Exercise:
    // - Control character (< 0x20) unescaped in the middle of a string
    inp = '"foo\u0013"';
    jsonClipTest(inp, 'json');  // won't parse
    jsonClipTest(inp, 'jx');
    jsonClipTest(inp, 'jc');

    // Exercise:
    // - JX escapes
    inp = '"foo\\x00\\xff\\Udeadbeefbar"';
    jsonClipTest(inp, 'jx');

    // Exercise:
    // - JX custom syntax
    inp = '{ ' +
          'plainkey: true, ' +
          'buffer: |deadbeef|, ' +
          'pointer: (0xdeadbeef), ' +
          'numbers: [ Infinity, -Infinity, NaN ], ' +
          'dummy: "value" ' +
          '}';
    jsonClipTest(inp, 'jx');  // causes TypeErrors for decode failures
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
