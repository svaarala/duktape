/*
 *  ES2015 \u{H+} Unicode escape in RegExps.
 */

/*---
{
    "custom": true
}
---*/

/*===
regexp test
SyntaxError
===*/

function regexpTest() {
    var re;

    // Without '/u' Unicode mode the \u{H+} escape form is not supported.
    // V8 parses the \u literally so that the regexp is matched like
    // /u{41}/.  Duktape currently rejects this form.

    try {
        re = new RegExp('\\u{4}');
        print(re.test('uuu'));  // false
        print(re.test('uuuu')); // true
    } catch (e) {
        print(e.name);
    }

/* XXX: Remaining tests disabled because Unicode mode not supported yet for
   regexps.

    // Simple case for 'A' character.
    re = /foo\u0041\u{41}\u{041}\u{0041}\u{00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000041}bar/;
    print(re.test('fooAAAAAbar'));

    // Non-BMP is converted into surrogate pairs during lexing so will match
    // surrogate pair string.  The non-bmp character is treated exactly as if
    // the RegExp source contained a surrogate pair instead.  This may not be
    // correct but a useful first approximation.
    re = /foo\u{1f4a9}{2}bar/;
    print(re.test('foo\ud83d\udca9\u{1f4a9}bar'));

    // Non-BMP codepoints in character ranges is not currently handled correctly:
    // the codepoints are not converted to surrogate pairs (which wouldn't make
    // sense in a range) but the matching code doesn't combine surrogate pairs
    // either.  Intentionally no test yet.

    // RegExp source code displays escapes as is (currently), they're not
    // normalized.
    print(/foo\u{41}\u{0000000041}bar/);
*/
}

try {
    print('regexp test');
    regexpTest();
} catch (e) {
    print(e.stack || e);
}
