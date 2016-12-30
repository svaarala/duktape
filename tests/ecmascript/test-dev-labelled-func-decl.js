/*
 *  Labelled function statement in non-strict and strict mode:
 *  http://www.ecma-international.org/ecma-262/6.0/index.html#sec-labelled-statements
 */

/*===
foo called (non-strict)
SyntaxError
===*/

function test() {
    // A function declaration as part of a labelled statement is not technically
    // "at the top level" so it should be rejected.  ES2015 Annex B allows this
    // for non-strict functions though, and Duktape follows that for real world
    // compatibility.
    try {
        eval(
            '(function nonStrictTest() {\n' +
            '    label:\n' +
            '        function foo() { print("foo called (non-strict)"); }\n' +
            '    foo();\n' +
            '})()\n'
        );
    } catch (e) {
        print(e.name);
    }

    // For strict functions a SyntaxError (early error) is required, even with
    // Annex B behavior.
    try {
        eval(
            '(function strictTest() {\n' +
            '    "use strict";\n' +
            '    label:\n' +
            '        function foo() { print("foo called (strict)"); }\n' +
            '    foo();\n' +
            '})()\n'
        );
    } catch (e) {
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
