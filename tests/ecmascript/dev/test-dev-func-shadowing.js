/*
 *  Various identifier shadowing cases
 */

/*===
function
undefined
function
inner foo
number
===*/

/* Function expression name is bound outside the function scope;
 * a single-identifier scope is created between the surrounding
 * scope and the function scope.
 *
 * Thus you can declare a variable or a function of the same name,
 * and it will hide the function name binding.
 */

try {
    /* Base case */
    eval("(function foo() { print(typeof foo); })();");
} catch (e) {
    print(e.name);
}

try {
    /* Var declaration shadows and is undefined, since variable
     * declaration happens on entry but assignment only when
     * statement is reached.
     */
    eval("(function foo() { print(typeof foo); var foo = 1; })();");
} catch (e) {
    print(e.name);
}

try {
     /* Function declaration shadows and is defined as the inner
      * function: function declarations happen on entry.
      */
    eval("(function foo() { print(typeof foo); foo(); function foo() { print('inner foo'); }; })();");
} catch (e) {
    print(e.name);
}

try {
    /* Argument shadows the function name binding, again simply
     * because the function name binding exists "outside" the
     * function scope.
     */
    eval("(function foo(foo) { print(typeof foo); })(1);");
} catch (e) {
    print(e.name);
}

/*===
123
function
inner func
1
1
10
===*/

/* An argument name shadows automatic 'arguments' binding */

try {
    eval("(function foo(arguments) { print(arguments); })(123);");
} catch (e) {
    print(e.name);
}

/* A function declaration inside the function shadows automatic 'arguments'
 * binding.
 */

try {
    eval("(function foo() { function arguments() { print('inner func'); }; print(typeof arguments); arguments(); })();");
} catch (e) {
    print(e.name);
}

/* A variable declaration does NOT shadow the automatic 'arguments' binding. */

try {
    eval("(function foo() { var arguments; print(arguments[0]); })(1,2)");
} catch (e) {
    print(e.name);
}

try {
    eval("(function foo() { print(arguments[0]); var arguments = 10; print(arguments); })(1,2)");
} catch (e) {
    print(e.name);
}
