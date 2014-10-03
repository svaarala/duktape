/*===
number 1
object
===*/

/* This parses as a block statement with a labeled statement
 * ("foo:") and an expression statement ("1").
 */

var t;

t = eval("{foo:1}");
print(typeof t, t);

/* Probable intended object literal. */

t = eval("({foo:1})");
print(typeof t);
