/*
 *  Test for currently supported ES2015 Section B.1.4 syntax, enabled by
 *  DUK_USE_ES6_REGEXP_SYNTAX.
 */

/*===
["{"]
["}"]
["{1234}"]
["]"]
SyntaxError
["$"]
===*/

function test() {
    var r;

    function test(re, inp) {
        try {
            var res = new RegExp(re).exec(inp);
            print(Duktape.enc('jx', res));
        } catch (e) {
            print(e.name);
        }
    }

    // literal curly braces
    test('{', 'abc{xyz');
    test('}', 'abc}xyz');
    test('{\\d+}', 'foo{1234}bar');

    // literal brackets
    test(']', 'foo]bar');
    test('[', 'foo[bar');  // left bracket not yet supported

    // escaping dollar sign
    test('\\$', 'foo$bar');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
