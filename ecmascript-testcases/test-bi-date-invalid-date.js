/*---
{
    "nonstandard": true
}
---*/

/*===
Invalid Date
===*/

/* The required value was not specified in E5 or E5.1; E6 draft
 * indicates "Invalid Date" is what is required, and V8 agrees:
 *
 *   http://people.mozilla.org/~jorendorff/es6-draft.html#sec-D
 */

print(new Date(Number.POSITIVE_INFINITY).toString());
