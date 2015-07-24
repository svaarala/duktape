/*---
{
    "nonstandard": true
}
---*/

/*===
Invalid Date
===*/

/* The required value was not specified in E5 or E5.1; E6 indicates
 * "Invalid Date" is what is required, and V8 agrees:
 *
 *   http://www.ecma-international.org/ecma-262/6.0/index.html#sec-D
 */

print(new Date(Number.POSITIVE_INFINITY).toString());
