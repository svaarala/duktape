/*
 *  Some 'arguments' binding tests
 */

/*===
object
number
object
false
object
===*/

/* A non-strict 'arguments' binding is writable but not deletable. */

function arguments_write() {
    print(typeof arguments);
    arguments = 1;
    print(typeof arguments);
}

function arguments_delete() {
    print(typeof arguments);
    print(delete arguments);
    print(typeof arguments);
}

try {
    arguments_write();
} catch (e) {
    print(e.name);
}

try {
    arguments_delete();
} catch (e) {
    print(e.name);
}

/*===
SyntaxError
SyntaxError
===*/

/* A strict 'arguments' binding is immutable.  It cannot be deleted nor can
 * it be assigned to; doing so would be a SyntaxError.
 *
 * XXX: how to test that the binding is actually immutable, as it is a
 * SyntaxError trying to delete it or assign to it?  Even an eval() call
 * does not work: the eval will be parsed in strict mode too.
 */

try {
    eval("function arguments_strict_delete() { 'use strict'; delete arguments; }");
} catch (e) {
    print(e.name);
}

try {
    eval("function arguments_strict_delete() { 'use strict'; arguments = 1; }");
} catch (e) {
    print(e.name);
}
