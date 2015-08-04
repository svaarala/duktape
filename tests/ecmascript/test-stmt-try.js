/*
 *  Try statement (E5 Section 12.14).
 */

/*===
===*/

/* XXX: test for control flow -> normal, canceling with continue, etc */

/*===
foo
Error
foo
try finished
foo
bar
foo
123 234
foo 234
123 234
function
string
function
===*/

/* The catch variable has a "let scope", i.e. it uses a temporary declarative
 * lexical environment which is only active for the duration of the catch.
 * A previous normal variable declaration of the same name is shadowed,
 * temporarily.
 */

try {
    /* -> foo, Error, foo, try finished */
    eval("var e='foo'; print(e);\n" +
         "try { throw new Error('error') } catch (e) { print(e.name) };\n" +
         "print(e);");
    print("try finished");
} catch (e) {
    print(e.name);
}

try {
    /* multiple shadowing -> foo, bar, foo */
    eval("try { throw 'foo' } catch (e) { print(e); try { throw 'bar' } catch (e) { print(e) }; print(e) }");
} catch (e) {
    print(e.name);
}

try {
    /* shadow arguments temporarily */
    eval("(function (x, y) { print(x, y); try { throw 'foo' } catch (x) { print(x, y); }; print(x, y); })(123, 234)");
} catch (e) {
    print(e.name);
}

try {
    /* shadow function declarations temporarily */
    eval("(function (x, y) { function fn() {} print(typeof fn); try { throw 'foo' } catch (fn) { print(typeof fn) }; print(typeof fn); })(123, 234)");
} catch (e) {
    print(e.name);
}

/*===
10
catch-binding-3
123
123
===*/

/* The catch variable scope may be accessible through a function expression
 * created inside the catch clause.  The closure may persist indefinitely.
 * One can even eval() code from inside a returned scope!
 */

function createCatchScopeFunctions() {
    var res = [];
    var i;

    for (i = 0; i < 10; i++) {
        try {
            throw 'catch-binding-' + i;
        } catch (e) {
            res.push(function myfunc(code) {
                return eval(code);
            });
        }
    }

    return res;
}

function testCatchScope() {
    var funcs = createCatchScopeFunctions();

    // 'i' is not 'let bound' so it is 10 here
    print(funcs[3]('i'));

    // 'e' is 'let' bound
    print(funcs[3]('e'));

    // we can assign to 'i'
    print(funcs[3]('i=123'));

    // and read it back through another closure
    print(funcs[7]('i'));
}

try {
    testCatchScope();
} catch (e) {
    print(e.stack || e);
}

/*===
SyntaxError
SyntaxError
SyntaxError
try
finally
try finished
===*/

/* The try-catch-finally statement syntax uses Block (instead of Statement)
 * in its syntax for each of the parts.  In other words, the parts cannot
 * consist of a single statement and MUST begin with an opening curly brace.
 * Otherwise, SyntaxError.
 */

try {
    /* try-part is not a Block -> SyntaxError */
    eval("try\n" +
         "  print('try');\n" +
         "catch (e)\n" +
         "  { print('catch'); }\n" +
         "finally\n" +
         "  { print('finally'); }");
    print("never here");
} catch (e) {
    print(e.name);
}

try {
    /* catch-part is not a Block -> SyntaxError */
    eval("try\n" +
         "  { print('try'); }\n" +
         "catch (e)\n" +
         "  print('catch');\n" +
         "finally\n" +
         "  { print('finally'); }");
    print("never here");
} catch (e) {
    print(e.name);
}

try {
    /* finally-part is not a Block -> SyntaxError */

    /* Note: 'rhino' rejects a non-Block Statement in try and catch
     * parts but allows it here in the finally part.
     */

    eval("try\n" +
         "  { print('try'); }\n" +
         "catch (e)\n" +
         "  { print('catch'); }\n" +
         "finally\n" +
         "  print('finally');");
    print("never here");
} catch (e) {
    print(e.name);
}

try {
    /* all parts are Blocks -> 'try' and 'finally' printed */
    eval("try\n" +
         "  { print('try'); }\n" +
         "catch (e)\n" +
         "  { print('catch'); }\n" +
         "finally\n" +
         "  { print('finally'); }");
    print("try finished");
} catch (e) {
    print(e.name);
}

/*===
SyntaxError
try
try finished
try
try finished
SyntaxError
SyntaxError
===*/

/* Catch variable name cannot be a reserved word.  In strict mode, it also
 * cannot be 'eval' or arguments'.
 */

try {
    eval("try { print('try'); } catch (if) { print('catch'); }");
    print("try finished");
} catch (e) {
    print(e.name);
}

try {
    eval("try { print('try'); } catch (eval) { print('catch'); }");
    print("try finished");
} catch (e) {
    print(e.name);
}

try {
    eval("try { print('try'); } catch (arguments) { print('catch'); }");
    print("try finished");
} catch (e) {
    print(e.name);
}

try {
    eval("'use strict'; try { print('try'); } catch (eval) { print('catch'); }");
    print("try finished");
} catch (e) {
    print(e.name);
}

try {
    eval("'use strict'; try { print('try'); } catch (arguments) { print('catch'); }");
    print("try finished");
} catch (e) {
    print(e.name);
}

/*===
false
foo
foo
bar
===*/

/* The catch variable is a non-deletable mutable binding.  Attempt to
 * delete the variable should fail ('delete' evaluates to false).
 */

try {
    /* non-deletable */
    eval("try { throw 'foo' } catch (e) { print(delete(e)); print(e); }");
} catch (e) {
    print(e.name);
}

try {
    /* mutable */
    eval("try { throw 'foo' } catch (e) { print(e); e = 'bar'; print(e); }");
} catch (e) {
    print(e.name);
}
