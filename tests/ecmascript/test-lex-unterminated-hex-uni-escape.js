/*===
\u
1 u
\u1
2 u1
\u12
3 u12
\u123
4 u123
\u123G
5 u123G
\x
1 x
\x1
2 x1
\x1G
3 x1G
===*/

/* Unterminated hex or unicode escapes should apparently (?) be parsed with
 * the longest match principle at least inside string literals.
 *
 * In other words, if the string ends or the escape is otherwise invalid,
 * the escape characters should be parsed as itself: "\uX" -> "uX".
 *
 * Both V8 and Rhino parse these without a SyntaxError.
 */

function testString(x) {
    var t;

    print(x);
    var t = eval('"' + x + '"');
    print(t.length, t);
}

function testStrings() {
    var strings = [
        '\\u',
        '\\u1',
        '\\u12',
        '\\u123',
        '\\u123G',
        '\\x',
        '\\x1',
        '\\x1G',
    ];
    var i;

    for (i = 0; i < strings.length; i++) {
        try {
            testString(strings[i]);
        } catch (e) {
            print(e);
        }
    }
}


try {
    testStrings();
} catch (e) {
    print(e.name);
}

/*===
SyntaxError
===*/

/* However, an unterminated unicode escape outside a string literal
 * won't be parsed by V8 or Rhino if it is broken.
 */

try {
    // if "\u" parsed as "u", same as assignemtn:  u123 = 1
    print(eval('\\u123 = 1;'));
} catch (e) {
    print(e.name);
}
