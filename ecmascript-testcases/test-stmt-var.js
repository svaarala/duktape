/*
 *  Variable statement (E5 Section 12.2).
 */

/* XXX: Add semantics tests in various contexts (strict eval, non-strict eval,
 * global code, function, etc.
 */

/* Test value for scope tests. */
var globalValue = 'this is global';

/* Test variable declaration inside a dummy function. */
function testDeclFunc(decl) {
    var func;
    try {
        func = eval('(function dummy() {\n' + decl + '\n})');
        print(JSON.stringify(decl) + ' -> success');
        try {
            print('func call -> ' + func());
        } catch (e) {
            print('func call -> ' + e.name);
        }
    } catch (e) {
        print(JSON.stringify(decl) + ' -> ' + e.name);
    }
}

/*===
"var;" -> SyntaxError
"var x;" -> success
func call -> undefined
"var x = 123;" -> success
func call -> undefined
"var x,y;" -> success
func call -> undefined
"var x = 123, y=234;" -> success
func call -> undefined
"var x, y = 234,z,w=345, foo = 456, bar, quux;" -> success
func call -> undefined
"var\n  x = 123\n ,y = 234;" -> success
func call -> undefined
"var\n  x = 123\n ,y = 234\nprint(\"howdy\");\nreturn \"foo\";" -> success
howdy
func call -> foo
"var x = globalValue, y = x; print(\"x:\", x, \"y:\", y);" -> success
x: this is global y: this is global
func call -> undefined
"var x = globalValue; var y = x; print(\"x:\", x, \"y:\", y);" -> success
x: this is global y: this is global
func call -> undefined
"var y = x, x = globalValue; print(\"x:\", x, \"y:\", y);" -> success
x: this is global y: undefined
func call -> undefined
"var y = x; var x = globalValue; print(\"x:\", x, \"y:\", y);" -> success
x: this is global y: undefined
func call -> undefined
"var x = globalValue; print(\"x:\", x, \"y:\", y); var y = x;" -> success
x: this is global y: undefined
func call -> undefined
"var x = globalValue; print(\"x:\", x, \"globalValue:\", globalValue); var globalValue = \"shadow\";" -> success
x: undefined globalValue: undefined
func call -> undefined
"var x = globalValue; var globalValue = \"shadow\"; print(\"x:\", x, \"globalValue:\", globalValue);" -> success
x: undefined globalValue: shadow
func call -> undefined
"var globalValue = \"shadow\"; var x = globalValue; print(\"x:\", x, \"globalValue:\", globalValue);" -> success
x: shadow globalValue: shadow
func call -> undefined
===*/

function variableDeclarationTest() {
    var test = testDeclFunc;

    /* Variable declaration cannot have zero variable. */
    test('var;');

    /* Various lengths, with and without initializer, various whitespace. */
    test('var x;');
    test('var x = 123;');
    test('var x,y;');
    test('var x = 123, y=234;');
    test('var x, y = 234,z,w=345, foo = 456, bar, quux;');

    /* Newlines within declaration are OK. */
    test('var\n' +
         '  x = 123\n' +
         ' ,y = 234;');

    /* Semicolon insertion. */
    test('var\n' +
         '  x = 123\n' +
         ' ,y = 234\n' +
         'print("howdy");\n' +
         'return "foo";');

    /* Variable initializers are executed in sequence.  All bindings in
     * the function exist from the beginning and have 'undefined' values
     * until assigned to.  All bindings in the a function shadow any outer
     * values of the same name.
     */

    test('var x = globalValue, y = x; print("x:", x, "y:", y);');
    test('var x = globalValue; var y = x; print("x:", x, "y:", y);');

    test('var y = x, x = globalValue; print("x:", x, "y:", y);');
    test('var y = x; var x = globalValue; print("x:", x, "y:", y);');

    test('var x = globalValue; print("x:", x, "y:", y); var y = x;');

    test('var x = globalValue; print("x:", x, "globalValue:", globalValue); var globalValue = "shadow";');
    test('var x = globalValue; var globalValue = "shadow"; print("x:", x, "globalValue:", globalValue);');
    test('var globalValue = "shadow"; var x = globalValue; print("x:", x, "globalValue:", globalValue);');
}

try {
    variableDeclarationTest();
} catch (e) {
    print(e.stack || e);
}
