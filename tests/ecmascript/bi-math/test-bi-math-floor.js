/*
 *  Math.floor()
 */

/*@include util-number.js@*/

/*===
123
122
-123
-124
4294967306
NaN
0
-0
Infinity
-Infinity
0
1
===*/

printExact(Math.floor(123));
printExact(Math.floor(122.999999));
printExact(Math.floor(-123));
printExact(Math.floor(-124));

printExact(Math.floor(4294967306.5));  // value higher than 32 bits

printExact(Math.floor(NaN));
printExact(Math.floor(+0));
printExact(Math.floor(-0));
printExact(Math.floor(Number.POSITIVE_INFINITY));
printExact(Math.floor(Number.NEGATIVE_INFINITY));
printExact(Math.floor(0.5));
printExact(Math.floor(1));
