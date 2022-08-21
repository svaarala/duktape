/*
 *  Addition precedence test.
 */

/*===
3foo34
===*/

/* This is equivalent to: (((1 + 2) + 'foo') + 3) + 4 === "3foo34" */

print(1 + 2 + 'foo' + 3 + 4);
