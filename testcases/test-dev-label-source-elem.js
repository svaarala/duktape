/*
 *  A labelled statement can only be followed by a statement, not a
 *  source element, even at the top level.  Concretely, a function
 *  declaration cannot follow a label, nor can an expression statement
 *  begin with 'function'.
 *
 *  This is the E5 standards compliant behavior, which this test now
 *  looks for.  In practice, it makes sense to parse these as either
 *  function expressions, or as some form of 'function statements'.
 *  The test needs to be updated when decision on this is made.
 */

/*===
SyntaxError
SyntaxError
try finished
===*/

try {
    /* Function f2 declaration follows label which is not allowed.
     * Also, an expression statement cannot begin with 'function',
     * so this should result in SyntaxError.
     *
     * V8 allows this in non-strict mode (as function statement?).
     */

    eval("function f1() { mylabel: function f2() {} }");
    print("try finished");
} catch (e) {
    print(e.name);
}

try {
    /* Strict mode should have no effect on this, but this test
     * illustrates V8 behavior (i.e. V8 gives a SyntaxError here
     * but only in strict mode).
     */
    eval("function f1() { 'use strict'; mylabel: function f2() {} }");
    print("try finished");
} catch (e) {
    print(e.name);
}

try {
    /* Here function f2 is a function expression, so it is OK. */
    eval("function f1() { mylabel: (function f2() {}) }");
    print("try finished");
} catch (e) {
    print(e.name);
}

