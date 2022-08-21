/*
 *  Empty alternatives match and capture empty groups.
 */

var t;

/*===
string string undefined
===*/

/* The first alternative will match and capture the empty string.
 * The second alternative is not tried and capture is undefined.
 */

t = /()|()/.exec('');
print(typeof t[0], typeof t[1], typeof t[2]);
