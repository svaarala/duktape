/*
 *  Some arguments shadowing test cases.
 */

/*===
formal argument
number 2
variable
object [object Arguments] arg1
number 5
function declaration
function
catch clause (does not shadow)
number 123
object [object Arguments] arg1
same name (non-strict)
2
2
20
same name (strict)
SyntaxError
===*/

print('formal argument');
function f1(a, arguments) {
    print(typeof arguments, arguments);
}
try {
    f1(1,2);
} catch (e) {
    print(e);
}

print('variable');
function f2() {
    print(typeof arguments, arguments, arguments[0]);  // 'arguments' value not shadowed here
    var arguments = 5;
    print(typeof arguments, arguments);
}
try {
    f2('arg1');
} catch (e) {
    print(e);
}

print('function declaration');
function f3() {
    function arguments() {}
    print(typeof arguments);
}
try {
    f3();
} catch (e) {
    print(e);
}

print('catch clause (does not shadow)');
function f4() {
    try {
        throw 123;
    } catch(arguments) {
        // arguments temporarily shadowed here
        print(typeof arguments, arguments);
    }
    print(typeof arguments, arguments, arguments[0]);
}
try {
    f4('arg1');
} catch (e) {
    print(e);
}

print('same name (non-strict)');
function f5(a,a) {
    print(a);  // prints '2'

    // arguments[0] is not magically bound
    // arguments[1] is magically bound to 'a'

    arguments[0] = 10;
    print(a);  // prints '2'

    arguments[1] = 20;
    print(a);  // prints '20'
}
try {
    f5(1,2);
} catch (e) {
    print(e);
}

print('same name (strict)');
function f6() {
    // Cannot test the actual behavior in strict mode because multiple
    // arguments of the same name is a SyntaxError.

    eval('function strict_test(a,a) { "use strict" }');
    'use strict';  // strict -> no magic bindings
}
try {
    f6();
} catch (e) {
    print(e.name);
}
