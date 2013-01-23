/*===
A
===*/

/* This was broken at some point, returning 65 (argument) instead of
 * desired value, string "A".
 */

function f(x) {
    return String.fromCharCode(x);
}

try {
    print(f(65));
} catch (e) {
    print(e.name);
}
