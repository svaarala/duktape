/*
 *  Math.acos()
 */

/*@include util-number.js@*/

/*===
1570796
0
3141593
NaN
NaN
NaN
===*/

printRounded6(Math.acos(0));
printExact(Math.acos(1));
printRounded6(Math.acos(-1))
printRounded6(Math.acos(Number.NaN));
printRounded6(Math.acos(1.0000001));
printRounded6(Math.acos(-1.0000001));
