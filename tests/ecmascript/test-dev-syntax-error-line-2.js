/*
 *  Since Duktape 1.2.0 SyntaxErrors have an explicit fileName/lineNumber
 *  property.
 */

/*---
{
    "custom": true
}
---*/

/*===
input 1
input 2
input 4
===*/

function test(inp) {
    try {
        eval(inp);
        print('never here');
    } catch (e) {
        //print(e.stack);
        print(e.fileName, e.lineNumber);
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
