/*
 *  Math.cos()
 */

/*@include util-number.js@*/

/*===
NaN
1
1
NaN
NaN
540302
===*/

printExact(Math.cos(NaN));
printExact(Math.cos(+0));
printExact(Math.cos(-0));
printExact(Math.cos(Number.POSITIVE_INFINITY));
printExact(Math.cos(Number.NEGATIVE_INFINITY));
printRounded6(Math.cos(1));
