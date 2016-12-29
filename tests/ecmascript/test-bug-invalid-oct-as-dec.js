/*===
63
78
88
99
77
78
88
99
===*/

/* Both V8, Spidermonkey, and Rhino allow "invalid" octal constants to be
 * parsed as decimal values.  Since this seems like dominant real world
 * behavior, Duktape also allows it.
 */

function octalTest() {
    function e(x) {
        try {
            print(eval(x));
        } catch (e) {
            print(e.name);
        }
    }
    function p(x) {
        try {
            print(parseInt(x));
        } catch (e) {
            print(e.name);
        }
    }

    // In source code, invalid octals are parsed as decimal (e.g. 088 -> 88)
    // by V8 and Rhino.

    e('077');
    e('078');
    e('088');
    e('099');

    // Older V8 versions would parseInt('088') as 0 and parseInt('078') as 7,
    // treating the non-octal parts as garbage.  Newer V8 parses all the cases
    // as decimal, i.e. 88 and 78, and parseInt('077') is also 77 (not 63).
    // This seems to be ES2015 behavior and Duktape also follows it.

    p('077');
    p('078');
    p('088');
    p('099');
}

try {
    octalTest();
} catch (e) {
    print(e);
}
