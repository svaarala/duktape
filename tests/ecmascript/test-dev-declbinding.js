/*
 *  Tests for declaration binding instantiation (E5 Section 10.5).
 */

/*===
1
false
1
SyntaxError
1
false
1
SyntaxError
function
false
function
SyntaxError
===*/

/* Argument, function declaration, and variable bindings are
 * non-configurable (non-deletable) in function code.
 */

try {
    // binding not deletable, 'delete a' returns false (E5 Section 10.2.1.1.5,
    // step 3).
    eval("function func_arg1(a) { print(a); print(delete a); print(a); }");
    func_arg1(1);
} catch (e) {
    print(e.name);
}

try {
    // in strict mode, SyntaxError
    eval("function func_arg2(a) { 'use strict'; print(a); print(delete a); print(a); }");
    func_arg2(1);
} catch (e) {
    print(e.name);
}

try {
    eval("function func_var1() { var a = 1; print(a); print(delete a); print(a); }");
    func_var1();
} catch (e) {
    print(e.name);
}

try {
    eval("function func_var2() { 'use strict'; var a = 1; print(a); print(delete a); print(a); }");
    func_var2();
} catch (e) {
    print(e.name);
}

try {
    eval("function func_fun1() { function a() {}; print(typeof a); print(delete a); print(typeof a); }");
    func_fun1();
} catch (e) {
    print(e.name);
}

try {
    eval("function func_fun2() { 'use strict'; function a() {}; print(typeof a); print(delete a); print(typeof a); }");
    func_fun2();
} catch (e) {
    print(e.name);
}

/*===
10
true
ReferenceError
function
true
undefined
===*/

/* Variable and function declarations established using a direct eval ARE deletable,
 * even in function code.
 */

try {
    eval("function func_var_eval1() { eval('var a = 10'); print(a); print(delete(a)); print(a); };");
    func_var_eval1();
} catch (e) {
    print(e.name);
}

try {
    eval("function func_fun_eval1() { eval('function a() {}'); print(typeof a); print(delete(a)); print(typeof a); };");
    func_fun_eval1();
} catch (e) {
    print(e.name, e.message);
}

/*===
true
SyntaxError
===*/

/* Attempt to delete an unresolvable identifier reference returns true (!) in
 * non-strict mode, and is a SyntaxError in strict mode.
 */

try {
    eval("function func_unres1(a) { print(delete unresolvable); }");
    func_unres1(1);
} catch (e) {
    print(e.name);
}

try {
    eval("function func_unres2(a) { 'use strict'; print(delete unresolvable); }");
    func_unres2(1);
} catch (e) {
    print(e.name);
}

/*===
undefined
undefined
object
===*/

/* Arguments binding should exist for functions, not for global/eval code.
 *
 * Note: 'typeof foo' where 'foo' is an undefined variable is NOT a
 * ReferenceError but returns undefined.  See E5 Section 11.4.3, step 2.a.
 *
 * Note: Rhino has 'arguments' bound in global code.
 */

try {
    // global
    print(typeof arguments);
} catch (e) {
    print(e.name);
}

try {
    // eval embedded in global
    print(eval("typeof arguments"));
} catch (e) {
    print(e.name);
}

try {
    // function
    eval("function foo() { print(typeof arguments); }; foo();");
} catch (e) {
    print(e.name);
}

/*===
object
number
SyntaxError
===*/

/* An arguments binding is mutable for non-strict code, immutable for strict code.
 *
 * Attempt to assign to arguments is a SyntaxError in strict code, so this is
 * actually pretty difficult to test :-).  Even direct eval in strict code
 * creates a separate variable environment.
 */

function arguments_mutability1() {
    print(typeof arguments);
    arguments = 1;
    print(typeof arguments);
}

try {
    arguments_mutability1();
} catch (e) {
    print(e.name);
}

try {
    eval("function arguments_mutability2() {\n" +
         "    'use strict';\n" +
         "    print(typeof arguments);\n" +
         "    arguments = 1;\n" +
         "    print(typeof arguments);\n" +
         "}");
} catch (e) {
    print(e.name);
}
