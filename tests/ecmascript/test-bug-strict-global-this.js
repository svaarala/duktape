/*
 *  If global code is strict it still gets a 'this' binding
 *  (not undefined as in Duktape 0.8.0).  This is apparent from
 *  E5.1 Section 10.4.1.
 */

"use strict";

/*===
object
===*/

try {
    print(typeof this);
} catch (e) {
    print(e);
}
