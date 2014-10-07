/* Function formal arguments do not accept reserved words.
 *
 * We interpret this also to mean that a function declaration/expression
 * in non-strict code will respect strict mode restricted reserved words
 * if the function body is strict.  This matches with V8.
 */

var orig_eval = eval;

/*===
SyntaxError
SyntaxError
success
SyntaxError
===*/

try {
    // Keyword
    eval("function foo(for) {};");
    print('never here');
} catch (e) {
    print(e.name);
}

try {
    // FutureReservedWord
    eval("function foo(class) {};");
    print('never here');
} catch (e) {
    print(e.name);
}

try {
    // FutureReservedWord only recognized in strict mode
    // -> should work
    eval("function foo(implements) { print(implements) }; foo('success');");
} catch (e) {
    print(e.name);
}

try {
    // FutureReservedWord only recognized in strict mode,
    // function declared in non-strict mode but function
    // itself is strict
    eval("function foo(implements) { 'use strict'; };");
    print('never here');
} catch (e) {
    print(e.name);
}

/*===
SyntaxError
SyntaxError
success
SyntaxError
===*/

/* Function expressions */

try {
    eval("(function foo(for) {})();");
    print('never here');
} catch (e) {
    print(e.name);
}

try {
    eval("(function foo(class) {})();");
    print('never here');
} catch (e) {
    print(e.name);
}

try {
    eval("(function foo(implements) { print(implements) })('success');");
} catch (e) {
    print(e.name);
}

try {
    eval("(function foo(implements) { 'use strict'; })();");
    print('never here');
} catch (e) {
    print(e.name);
}

/*===
success
SyntaxError
success
SyntaxError
===*/

/* Eval and arguments */

try {
    eval("function foo(eval) { print(eval); }; foo('success');");
} catch (e) {
    print(e.name);
}

try {
    eval("function foo(eval) { 'use strict'; print(eval); }; foo('success');");
    print('never here');
} catch (e) {
    print(e.name);
}

try {
    eval("function foo(arguments) { print(arguments); }; foo('success');");
} catch (e) {
    print(e.name);
}

try {
    eval("function foo(arguments) { 'use strict'; print(arguments); }; foo('success');");
    print('never here');
} catch (e) {
    print(e.name);
}

/*===
success
SyntaxError
success
SyntaxError
===*/

/* Eval and arguments, function expressions */

try {
    eval("(function foo(eval) { print(eval); })('success');");
} catch (e) {
    print(e.name);
}

try {
    eval("(function foo(eval) { 'use strict'; print(eval); })('success');");
    print('never here');
} catch (e) {
    print(e.name);
}

try {
    eval("(function foo(arguments) { print(arguments); })('success');");
} catch (e) {
    print(e.name);
}

try {
    eval("(function foo(arguments) { 'use strict'; print(arguments); })foo('success');");
    print('never here');
} catch (e) {
    print(e.name);
}

/*===
2
SyntaxError
===*/

/* Duplicate argument names */

try {
    // ok, binds to latter
    eval("function foo(a,a) { print(a); }; foo(1,2);");
} catch (e) {
    print(e.name);
}

try {
    eval("function foo(a,a) { 'use strict'; print(a); }; foo(1,2);");
    print('never here');
} catch (e) {
    print(e.name);
}

/*===
2
SyntaxError
===*/

/* Duplicate argument names, function expressions */

try {
    // ok, binds to latter
    eval("(function foo(a,a) { print(a); })(1,2);");
} catch (e) {
    print(e.name);
}

try {
    eval("(function foo(a,a) { 'use strict'; print(a); })(1,2);");
    print('never here');
} catch (e) {
    print(e.name);
}
