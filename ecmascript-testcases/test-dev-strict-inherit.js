/*===
true
SyntaxError
true
===*/

/* An inner function or an eval call of a strict function is automatically
 * strict, even without an explicit use strict declaration.
 *
 * A strict function 'this' binding is undefined; a non-strict function has
 * the global object as its 'this' binding (when called as a plain function).
 * Thus, '!this' is true for strict code, and false for non-strict code.
 *
 * Also, 'delete' for an unresolvable identifier is a SyntaxError in strict
 * code but returns false in non-strict code.
 */

function outer_strict() {
    'use strict';
    var foo = 1;

    try {
        print(!this);
    } catch (e) {
        print(e.name);
    }

    try {
        print(eval("delete foo"));
    } catch (e) {
        print(e.name);
    }

    function inner() {
        try {
            print(!this);
        } catch (e) {
            print(e.name);
        }
    }

    inner();
}

outer_strict();
