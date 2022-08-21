/*
 *  Math.sin()
 */

/*@include util-number.js@*/

/*===
NaN
0
-0
NaN
NaN
841471
===*/

printExact(Math.sin(NaN));
printExact(Math.sin(+0));
printExact(Math.sin(-0));
printExact(Math.sin(Number.POSITIVE_INFINITY));
printExact(Math.sin(Number.NEGATIVE_INFINITY));
printRounded6(Math.sin(1));
