/*===
SyntaxError
===*/

/* This was broken at some point: formal argument list is parsed in
 * non-strict mode so 'implements' is allowed.  When function is
 * detected to be strict, the argument list needs to be rechecked
 * to ensure that there are no reserved words which are recognized
 * only in strict mode.
 *
 * This is not actually clear cut in the specification, but this
 * behavior follows e.g. V8.
 */

try {
    // FutureReservedWord only recognized in strict mode,
    // function declared in non-strict mode but function
    // itself is strict
    eval("function foo(implements) { 'use strict'; };");
    print('never here');
} catch (e) {
    print(e.name);
}
