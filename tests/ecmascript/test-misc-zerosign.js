/*
 *  A simple zero sign behavior test.
 *
 *  Comprehensive tests of zero sign behavior belong in e.g. arithmetic
 *  operation tests.
 */

/*===
Infinity number
-Infinity number
===*/

var t;

t = 1 / +0;
print(t, typeof t);

t = 1 / -0;
print(t, typeof t);
