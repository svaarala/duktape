/*
 *  Math.round()
 */

/*@include util-number.js@*/

/*===
NaN
0
-0
Infinity
-Infinity
0
1
-0
-1
4
-3
===*/

printExact(Math.round(NaN));
printExact(Math.round(+0));
printExact(Math.round(-0));
printExact(Math.round(Number.POSITIVE_INFINITY));
printExact(Math.round(Number.NEGATIVE_INFINITY));
printExact(Math.round(0.400009));
printExact(Math.round(0.5));
printExact(Math.round(-0.5));
printExact(Math.round(-0.500001));
printExact(Math.round(3.5));   // tie break towards +Infinity -> 4
printExact(Math.round(-3.5));  // tie break towards +Infinity -> -3
