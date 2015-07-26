/*===
undefined
===*/

/* 'x' does not resolve, should not cause a ReferenceError but
 * an "undefined" typeof.  E5 Section 11.4.3, step 2.a.
 *
 * This was broken at some point.
 */

try {
    print(typeof x);
} catch (e) {
    print(e.name);
}
