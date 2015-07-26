/*===
321
undefined
===*/

/* If an eval() call happens in a strict function, the eval code will be
 * considered strict even without an explicit strict declaration.
 *
 * A strict direct eval call gets a fresh variable environment so any
 * declarations will not be visible to the caller's env.
 */

function nonstrict_parent() {
    eval('var x = 321');   // new declaration, goes to parent
    print(x);
}

function strict_parent() {
    'use strict';

    eval('var x = 321');   // new declaration, goes to fresh env
    print(typeof x);       // -> undefined
}

try {
    nonstrict_parent();
} catch (e) {
    print(e.name);
}

try {
    strict_parent();
} catch (e) {
    print(e.name);
}
