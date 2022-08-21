/*
 *  Math.exp()
 */

/*@include util-number.js@*/

/*===
NaN
1
1
Infinity
0
===*/

printExact(Math.exp(NaN));
printExact(Math.exp(+0));
printExact(Math.exp(-0));
printExact(Math.exp(Number.POSITIVE_INFINITY));
printExact(Math.exp(Number.NEGATIVE_INFINITY));
