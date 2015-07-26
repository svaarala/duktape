/*===
false
true
===*/

/* Leading zeroes are allowed and the quantifier is still parsed as
 * decimal.
 */

r = /x{010,011}/;
print(r.test('xxxxxxxx'));

r = /x{010,011}/;
print(r.test('xxxxxxxxxx'));
