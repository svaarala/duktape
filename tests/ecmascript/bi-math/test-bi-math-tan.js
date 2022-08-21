/*
 *  Math.tan()
 */

/*@include util-number.js@*/

/*===
1557408
-2185040
648361
NaN
0
-0
NaN
NaN
===*/

printRounded6(Math.tan(1));
printRounded6(Math.tan(2));
printRounded6(Math.tan(10));

printExact(Math.tan(NaN));
printExact(Math.tan(+0));
printExact(Math.tan(-0));
printExact(Math.tan(Number.POSITIVE_INFINITY));
printExact(Math.tan(Number.NEGATIVE_INFINITY));
