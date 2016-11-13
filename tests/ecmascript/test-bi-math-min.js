/*
 *  Math.min()
 */

/*@include util-number.js@*/

/*===
-123
-Infinity
123
0
-0
Infinity
NaN
-0
-0
===*/

printExact(Math.min(123, -123, 456, 234));
printExact(Math.min(123, -123, Number.NEGATIVE_INFINITY, 234));
printExact(Math.min(123));

printExact(Math.min(+0));
printExact(Math.min(-0));

printExact(Math.min());  // -> Infinity (!)
printExact(Math.min(1,2,NaN,4));

printExact(Math.min(+0, -0));  // -> -0

printExact(Math.min(-0, +0));  // -> -0
