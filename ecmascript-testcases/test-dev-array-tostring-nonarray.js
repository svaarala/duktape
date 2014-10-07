/*===
JOIN
===*/

/* Array prototype functions are generic and can be used with objects
 * other than arrays.  Here, for instance, we use Array.prototype.toString()
 * on a custom object.
 *
 * Because the custom object has a 'join' function, it is called.
 */

var obj = { join: function() { return 'JOIN'; } };

print(Array.prototype.toString.apply(obj));
