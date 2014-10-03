/*===
0
0
0
===*/

/* The ToNumber() coercion yields zero from an empty string or
 * from a string consisting entirely of whitespace.  See E5
 * Section 9.3.1.
 *
 * Unary plus is an easy way to get a plain ToNumber() coercion.
 */

print(+"");
print(+" ");
print(+"\n\n");
