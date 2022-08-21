/*
 *  Just random tests for ensuring valstack does not grow during
 *  execution.
 */

/*===
finished
===*/

/* This was broken at one point: executor left temp values on valstack
 * which eventually ran out of slack.
 */

eval("for (var i = 0; i < 100000; i++) { 'foo' + i + 'bar' }");
print("finished");
