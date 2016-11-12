/*
 *  Math.atan()
 */

/*@include util-number.js@*/

/*===
NaN
0
-0
1570796
-1570796
===*/

printExact(Math.atan(Number.NaN));
printExact(Math.atan(+0));
printExact(Math.atan(-0));
printRounded6(Math.atan(Number.POSITIVE_INFINITY));
printRounded6(Math.atan(Number.NEGATIVE_INFINITY));
