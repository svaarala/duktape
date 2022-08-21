/*
 *  Math.atan2()
 */

/*@include util-number.js@*/

/*===
NaN
NaN
NaN
1570796
1570796
0
0
3141593
3141593
-0
-0
-3141593
-3141593
-1570796
-1570796
0
3141593
-0
-3141593
1570796
1570796
-1570796
-1570796
785398
2356194
-785398
-2356194
===*/

printExact(Math.atan2(NaN, NaN));
printExact(Math.atan2(1, NaN));
printExact(Math.atan2(NaN, 1));
printRounded6(Math.atan2(1, +0));
printRounded6(Math.atan2(1, -0));

printExact(Math.atan2(+0, 1));
printExact(Math.atan2(+0, +0));
printRounded6(Math.atan2(+0, -0));
printRounded6(Math.atan2(+0, -1));

printExact(Math.atan2(-0, 1));
printExact(Math.atan2(-0, +0));
printRounded6(Math.atan2(-0, -0));
printRounded6(Math.atan2(-0, -1));

printRounded6(Math.atan2(-1, +0));
printRounded6(Math.atan2(-1, -0));

printExact(Math.atan2(1, Number.POSITIVE_INFINITY));
printRounded6(Math.atan2(1, Number.NEGATIVE_INFINITY));
printExact(Math.atan2(-1, Number.POSITIVE_INFINITY));
printRounded6(Math.atan2(-1, Number.NEGATIVE_INFINITY));

printRounded6(Math.atan2(Number.POSITIVE_INFINITY, 1));
printRounded6(Math.atan2(Number.POSITIVE_INFINITY, -1));
printRounded6(Math.atan2(Number.NEGATIVE_INFINITY, 1));
printRounded6(Math.atan2(Number.NEGATIVE_INFINITY, -1));

printRounded6(Math.atan2(Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY));
printRounded6(Math.atan2(Number.POSITIVE_INFINITY, Number.NEGATIVE_INFINITY));
printRounded6(Math.atan2(Number.NEGATIVE_INFINITY, Number.POSITIVE_INFINITY));
printRounded6(Math.atan2(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY));
