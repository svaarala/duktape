/*===
true
false
===*/

/* The function constructed with Function constructor does not inherit
 * its parent's strictness.  Strictness is determined only based on
 * the function body itself (i.e., the presence of a 'use strict').
 */

function f() {
    'use strict';

    // non-strict: this=global -> !this=false
    var g = new Function('print(!this)');

    // strict: this=undefined -> !this=true
    print(!this);
    g();
}

try {
    f();
} catch (e) {
    print(e.name);
}
