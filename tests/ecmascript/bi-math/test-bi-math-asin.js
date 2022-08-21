/*
 *  Math.asin()
 */

/*@include util-number.js@*/

/*===
0
-0
1570796
-1570796
NaN
NaN
NaN
===*/

printExact(Math.asin(+0));
printExact(Math.asin(-0));
printRounded6(Math.asin(1));
printRounded6(Math.asin(-1))
printRounded6(Math.asin(Number.NaN));
printRounded6(Math.asin(1.0000001));
printRounded6(Math.asin(-1.0000001));
