/*
 *  Math.log()
 */

/*@include util-number.js@*/

/*===
4812184
NaN
NaN
NaN
-Infinity
-Infinity
0
Infinity
===*/

printRounded6(Math.log(123));

printExact(Math.log(NaN));

printExact(Math.log(-0.000001));
printExact(Math.log(Number.NEGATIVE_INFINITY));

printExact(Math.log(+0));
printExact(Math.log(-0));

printExact(Math.log(1));

printExact(Math.log(Number.POSITIVE_INFINITY));
