/*---
{
    "nonstandard": true
}
---*/

/*===
63
78
88
99
63
7
0
0
===*/

/* Both V8 and Rhino allow "invalid" octal constants to be parsed as decimal
 * values; we currently don't, but this might not be the desired behavior.
 * This bug testcases demonstrates the difference.
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
    // by V8 and Rhino

    e('077');
    e('078');
    e('088');
    e('099');

    // In parseInt(), Rhino and V8 differ.  V8 probably has most useful behavior,
    // parsing '088' and '099' as 0 (treating 88 and 99 as garbage), and '078'
    // as 7 (treating the trailing '8' as garbage).

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
