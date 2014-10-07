/*===
string
undefined
===*/

/* For an UnresolvableReference, typeof should still return a string
 * value "undefined".  (Current implementation returns an undefined
 * value.)
 */

try {
    var t = typeof foo;   // UnresolvableReference
    print(typeof t);
    print(t);
} catch (e) {
    print(e);
}
