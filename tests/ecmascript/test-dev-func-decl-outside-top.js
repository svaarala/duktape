/*
 *  Test behavior for function declaration outside top level.
 *
 *  If the non-standard syntax for function declaration outside top level is
 *  not supported at all, this testcase fails to parse with a SyntaxError.
 *  Wrapping the tests inside eval() would be awkward.
 *
 *  Duktape default behavior is modelled after V8.  The expect strings
 *  have also been set with that behavior in mind.
 */

/*---
{
    "nonstandard": true
}
---*/

// This should not get accessed by any test, but bind it to something
// typeof can distinguish.
var fun1 = 'outside';

/*===
standard function declaration
A function
fun1
B function
fun1
C number
TypeError
===*/

function standardFunctionDeclarationTest() {
    // Standard behavior: fun1 is bound already here
    print('A', typeof(fun1));
    fun1();  // works

    /* Standard case: fun1 is declared at top level, and from scoping point
     * of view exists at the top level (does not see e.g. try-catch bindings).
     * The 'fun1' identifier is accessible and bound to the actual function
     * at function entry.
     */
    function fun1() {
        print('fun1');
    }

    // Standard behavior: fun1 is of course also bound here
    print('B', typeof(fun1));
    fun1();  // works

    // Standard behavior: fun1 is assigned a number, call leads to TypeError
    fun1 = 123;
    print('C', typeof(fun1));
    try { fun1() } catch (f) { print(f.name); };
}

print('standard function declaration');

try {
    standardFunctionDeclarationTest();
} catch (e) {
    print(e);
}

/*===
standard function expression
A undefined
TypeError
B function
fun1 object
C function
fun1 object
D number
TypeError
===*/

function standardFunctionExpressionTest() {
    /* Function expression assigned dynamically to a variable. */

    var fun1;  // undefined at entry

    // Standard behavior: fun1 will be undefined, call will fail with TypeError
    print('A', typeof fun1);
    try { fun1(); } catch (f) { print(f.name); }

    try {
        throw new Error('test');
    } catch (e) {
        fun1 = function fun1() {
            // Function expression is given name 'fun1' so that it shows
            // up in tracebacks etc.
            print('fun1', typeof e);
        }

        // Standard behavior: fun1 is bound and has access to 'e'
        print('B', typeof fun1);
        fun1();
    }

    // Standard behavior: fun1 is still bound and has access to 'e'
    print('C', typeof fun1);
    fun1();

    // Standard behavior: fun1 is assigned a number, call leads to TypeError
    fun1 = 123;
    print('D', typeof(fun1));
    try { fun1() } catch (f) { print(f.name); };
}

print('standard function expression');

try {
    standardFunctionExpressionTest();
} catch (e) {
    print(e);
}

/*===
function declaration inside try (non-strict)
A function
fun1 undefined
B function
fun1 undefined
C function
fun1 undefined
D number
TypeError
===*/

function functionDeclarationInsideTryNonStrictTest() {
    /* Function declaration inside a try-catch block.  The test here is to
     * see how the function lexical environment works.  If the function sees
     * the 'catch' binding, it really cannot be accessible at function entry.
     */

    // Rhino: undefined (global 'fun1' not visible)
    // V8: function, 'e' not bound (-> undefined)
    print('A', typeof fun1);
    try { fun1(); } catch (f) { print(f.name); }

    try {
        throw new Error('test');
    } catch (e) {
        function fun1() {
            print('fun1', typeof e);
        }

        // Rhino: function (can see 'e' binding)
        // V8: function (cannot see 'e' binding!)
        print('B', typeof fun1);
        fun1();
    }

    // Rhino: function (can see 'e' binding)
    // V8: function (cannot see 'e' binding!)
    print('C', typeof fun1);
    fun1();

    // Rhino: fun1 is assigned a number, call leads to TypeError
    // V8: fun1 is assigned a number, call leads to TypeError
    fun1 = 123;
    print('D', typeof(fun1));
    try { fun1() } catch (f) { print(f.name); };
}

print('function declaration inside try (non-strict)');

try {
    functionDeclarationInsideTryNonStrictTest();
} catch (e) {
    print(e);
}

/*===
function declaration inside try (strict)
fun1 undefined
fun1 undefined
===*/

function functionDeclarationInsideTryStrictTest() {
    /* Same as above, but in strict mode.  V8 used to reject function
     * declarations outside top level in strict mode but now accepts
     * them with block level semantics.
     *
     * Rhino allows such declarations even in strict mode, and provides
     * the same semantics as in non-strict mode.
     */

    eval('(function () { ' +
             '"use strict";' +
             'try {' +
                 'throw new Error("test");' +
             '} catch (e) {' +
                 'function fun1() { print("fun1", typeof e); }' +
                 'fun1();' +  // V8: 'fun1 object'
             '}' +
             'fun1();' +  // V8: TypeError, fun1 not visible
         '})()');
}

print('function declaration inside try (strict)');

try {
    functionDeclarationInsideTryStrictTest();
} catch (e) {
    print(e.name);
}
