/*===
A
===*/

/* This was broken at some point, returning 65 (argument) instead of
 * desired value, string "A".
 */

function f(x) {
    return String.fromCharCode(x);
}

print(f(65));
