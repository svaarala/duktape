/*===
0 string 0
1 string 1
2 string 2
3 string length
===*/

/* The virtual 'length' property of a string object (as opposed to a
 * string value) was not included in getOwnPropertyNames() output.
 */

var obj = Object.getOwnPropertyNames(new String('foo'));
var i;

for (i = 0; i < obj.length; i++) {
    print(i, typeof obj[i], obj[i]);
}
