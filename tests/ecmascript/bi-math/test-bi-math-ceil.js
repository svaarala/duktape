/*
 *  Math.ceil()
 */

/*@include util-number.js@*/

/*===
123
124
-123
-122
4294967307
NaN
0
-0
Infinity
-Infinity
-0
-1
===*/

printExact(Math.ceil(123));
printExact(Math.ceil(123.000001));
printExact(Math.ceil(-123));
printExact(Math.ceil(-122.999999));

printExact(Math.ceil(4294967306.5));  // value higher than 32 bits

printExact(Math.ceil(NaN));
printExact(Math.ceil(+0));
printExact(Math.ceil(-0));
printExact(Number.POSITIVE_INFINITY);
printExact(Number.NEGATIVE_INFINITY);
printExact(Math.ceil(-0.5));
printExact(Math.ceil(-1.0));
