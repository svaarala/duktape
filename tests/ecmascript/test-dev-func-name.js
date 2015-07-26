/* Function name is an identifier (in both function declaration and
 * function expression) and cannot contain reserved words.  Also,
 * "eval" and "arguments" are prohibited for strict functions (although
 * they are not reserved words).
 *
 * We interpret (like V8) that a strict function contained in non-strict
 * code cannot be named with one of the additional strict mode reserved
 * words.
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
    eval("function for() {};");
    print('never here');
} catch (e) {
    print(e.name);
}

try {
    // FutureReservedWord
    eval("function class() {};");
    print('never here');
} catch (e) {
    print(e.name);
}

try {
    // FutureReservedWord only recognized in strict mode
    // -> should work
    eval("function implements() { print('success') }; implements();");
} catch (e) {
    print(e.name);
}

try {
    // FutureReservedWord only recognized in strict mode,
    // function declared in non-strict mode but function
    // itself is strict
    eval("function implements() { 'use strict'; };");
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

/* Same tests for function declarations */

try {
    eval("(function for() {});");
    print('never here');
} catch (e) {
    print(e.name);
}

try {
    eval("(function class() {});");
    print('never here');
} catch (e) {
    print(e.name);
}

try {
    eval("(function implements() { print('success') })();");
} catch (e) {
    print(e.name);
}

try {
    eval("(function implements() { 'use strict'; });");
    print('never here');
} catch (e) {
    print(e.name);
}

/*===
SyntaxError
SyntaxError
===*/

try {
    eval("function eval() { 'use strict'; };");
} catch (e) {
    print(e.name);
}

eval = orig_eval;  // just in case

try {
    eval("function arguments() { 'use strict'; };");
} catch (e) {
    print(e.name);
}

/*===
SyntaxError
SyntaxError
===*/

try {
    eval("(function eval() { 'use strict'; })();");
} catch (e) {
    print(e.name);
}

eval = orig_eval;  // just in case

try {
    eval("(function arguments() { 'use strict'; })();");
} catch (e) {
    print(e.name);
}

/*===
success
success
===*/

/* Non-strict mode allows eval() and arguments() named functions. */

try {
    eval("function eval(a) { print(a); }; eval('success');");
} catch (e) {
    print(e.name);
}

eval = orig_eval;  // just in case

try {
    eval("function arguments(a) { print(a); }; arguments('success');");
} catch (e) {
    print(e.name);
}

/*===
success
success
===*/

try {
    eval("(function eval(a) { print(a); })('success');");
} catch (e) {
    print(e.name);
}

eval = orig_eval;  // just in case

try {
    eval("(function arguments(a) { print(a); })('success');");
} catch (e) {
    print(e.name);
}
