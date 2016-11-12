/*
 *  Math.abs()
 */

/*@include util-number.js@*/

/*===
123
123
0
0
NaN
Infinity
Infinity
===*/

printExact(Math.abs(+123));
printExact(Math.abs(-123));
printExact(Math.abs(+0));
printExact(Math.abs(-0));
printExact(Math.abs(Number.NaN));
printExact(Math.abs(Number.POSITIVE_INFINITY));
printExact(Math.abs(Number.NEGATIVE_INFINITY));
