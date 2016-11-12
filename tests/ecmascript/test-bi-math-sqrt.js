/*
 *  Math.sqrt()
 */

/*@include util-number.js@*/

/*===
2000000
1414214
NaN
NaN
NaN
0
-0
Infinity
===*/

printRounded6(Math.sqrt(4));
printRounded6(Math.sqrt(2));

printExact(Math.sqrt(NaN));
printExact(Math.sqrt(-0.000001));
printExact(Math.sqrt(Number.NEGATIVE_INFINITY));
printExact(Math.sqrt(+0));
printExact(Math.sqrt(-0));
printExact(Math.sqrt(Number.POSITIVE_INFINITY));
