/*
 *  ES2015 Annex B allows regexp identity escape for any source code character
 *  except 'c'.  With DUK_USE_ES6_REGEXP_SYNTAX Duktape does too; test for
 *  a few basic cases.
 */

/*===
true
true
8232 plain SyntaxError
8232 charclass SyntaxError
8233 plain SyntaxError
8233 charclass SyntaxError
===*/

function test() {
    var re;
    var i;

    // Original reported issue: https://github.com/svaarala/duktape/issues/643
    try {
        re = eval('/[\\' + String.fromCharCode(198) + ']/');
        print(re.test('\u00c6'));
    } catch (e) {
        print(e.name);
    }

    // Same case outside of a character class.
    try {
        re = eval('/\\' + String.fromCharCode(198) + '/');
        print(re.test('\u00c6'));
    } catch (e) {
        print(e.name);
    }

    // Test BMP above ASCII range.  Everything is accepted except U+2028
    // and U+2029 which are treated like newlines and cause a RegExp syntax
    // error (V8 has the same behavior).
    for (i = 0x0080; i <= 0xffff; i++) {
        try {
            re = eval('/\\' + String.fromCharCode(i) + '/');
            if (re.test(String.fromCharCode(i)) !== true) {
                print(i, 'plain', 'no match');
            }
        } catch (e) {
            print(i, 'plain', e.name);
        }

        try {
            re = eval('/[\\' + String.fromCharCode(i) + ']/');
            if (re.test(String.fromCharCode(i)) !== true) {
                print(i, 'charclass', 'no match');
            }
        } catch (e) {
            print(i, 'charclass', e.name);
        }
    }

    // As of Duktape 2.0 there are still some differences for the ASCII range
    // because Duktape won't backtrack on e.g. an invalid hex escape to treat
    // the characters literally.
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
