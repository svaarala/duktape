/*
 *  SyntaxErrors thrown during parsing get their message augmented with a
 *  line indicating where the syntax error occurred.
 */

/*---
{
    "custom": true
}
---*/

/*===
line 1
line 2
line 4
===*/

function test(inp) {
    try {
        eval(inp);
        print('never here');
    } catch (e) {
        // Match against current syntax
        res = /^.*\(line (\d+), end of input\)$/.exec(e.message);
        if (!res) {
            res = /^.*\(line (\d+)\)$/.exec(e.message);
        }
        print('line', res ? res[1] : 'n/a');
    }
}

try {
    // Test for specific line numbers; these may change at some point due to compiler details
    test('"foo');
    test('\n"foo');
    test('\n\n1\n+');
} catch (e) {
    print(e);
}
