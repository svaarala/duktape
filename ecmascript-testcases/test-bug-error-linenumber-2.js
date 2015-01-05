/*
 *  In a throw statement the error linenumber will come potentially far
 *  after the error throwing line.  This is usually not an issue but
 *  may sometimes be quite confusing.  For example, automatic semicolon
 *  insertion affects the current behavior in a non-intuitive manner.
 */

"use strict";

/*===
Error 2
Error 2
line 2
TypeError 10
line 2
TypeError 10
===*/

function evalCheck(code) {
    try {
        eval(code);
        print('successful eval (unexpected)');
    } catch (e) {
        print(e.name + ' ' + e.lineNumber);
    }
}

function test() {
    /* Here the "throw" line is separated by empty lines from a following
     * print statement.  Right now, the "throw" line gets attributed
     * if there is a semicolon after the right, while the print" line
     * gets attributed if there is!
     */

    // -> 2, OK
    evalCheck("\nthrow new Error('at line 2');\n\n\n\n\n\n\n\nprint('at line 10');");

    // -> 10, not OK, should be 2
    evalCheck("\nthrow new Error('at line 2')\n\n\n\n\n\n\n\nprint('at line 10');");

    /* Here TypeError should be thrown from line 10, with and without
     * semicolon (works ok).
     */

    evalCheck("\nprint('line 2');\n\n\n\n\n\n\n\nthis.nonexistent.foo = 1;");
    evalCheck("\nprint('line 2')\n\n\n\n\n\n\n\nthis.nonexistent.foo = 1;");
}

try {
    test();
} catch (e) {
    print(e);
}
