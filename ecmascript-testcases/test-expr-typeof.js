/*
 *  'typeof' operator (E5 Section 11.4.3).
 */

/*===
undefined
object
boolean
boolean
number
number
number
number
string
object
function
function
function
===*/

print(typeof undefined);
print(typeof null);  // note: 'object' is correct
print(typeof true);
print(typeof false);
print(typeof 123);
print(typeof Number.NaN);
print(typeof Number.POSITIVE_INFINITY);
print(typeof Number.NEGATIVE_INFINITY);
print(typeof 'foo');
print(typeof {});     // object and not callable
print(typeof print);  // object and callable
print(typeof print.bind(null, 1, 2)); // object and callable (bound function)
print(typeof function() {});
