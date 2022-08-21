/*
 *  Math.max()
 */

/*@include util-number.js@*/

/*===
456
Infinity
123
0
-0
-Infinity
NaN
0
0
===*/

printExact(Math.max(123, -123, 456, 234));
printExact(Math.max(123, -123, Number.POSITIVE_INFINITY, 234));
printExact(Math.max(123));

printExact(Math.max(+0));
printExact(Math.max(-0));

printExact(Math.max());  // -> -Infinity (!)
printExact(Math.max(1,2,NaN,4));

printExact(Math.max(+0, -0));  // -> +0

printExact(Math.max(-0, +0));  // -> +0
