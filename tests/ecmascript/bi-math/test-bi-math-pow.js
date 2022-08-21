/*
 *  Math.pow()
 */

/*@include util-number.js@*/

/*===
pow 1
NaN
NaN
NaN
NaN
NaN
NaN
1
1
1
1
1
1
1
1
1
1
1
1
1
1
NaN
NaN
NaN
NaN
===*/

print('pow 1');

printExact(Math.pow(Number.NEGATIVE_INFINITY, NaN));
printExact(Math.pow(-1, NaN));
printExact(Math.pow(-0, NaN));
printExact(Math.pow(+0, NaN));
printExact(Math.pow(1, NaN));
printExact(Math.pow(Number.POSITIVE_INFINITY, NaN));

printExact(Math.pow(Number.NEGATIVE_INFINITY, +0));
printExact(Math.pow(-1, +0));
printExact(Math.pow(-0, +0));
printExact(Math.pow(+0, +0));
printExact(Math.pow(1, +0));
printExact(Math.pow(Number.POSITIVE_INFINITY, +0));
printExact(Math.pow(NaN, +0));

printExact(Math.pow(Number.NEGATIVE_INFINITY, -0));
printExact(Math.pow(-1, -0));
printExact(Math.pow(-0, -0));
printExact(Math.pow(+0, -0));
printExact(Math.pow(1, -0));
printExact(Math.pow(Number.POSITIVE_INFINITY, -0));
printExact(Math.pow(NaN, -0));

printExact(Math.pow(NaN, Number.NEGATIVE_INFINITY));
printExact(Math.pow(NaN, -1));
printExact(Math.pow(NaN, +1));
printExact(Math.pow(NaN, Number.POSITIVE_INFINITY));

/*===
pow 2
Infinity
Infinity
0
0
NaN
NaN
NaN
NaN
0
0
Infinity
Infinity
Infinity
0
-Infinity
Infinity
Infinity
Infinity
-0
0
0
0
===*/

print('pow 2');

printExact(Math.pow(1.0000001, Number.POSITIVE_INFINITY));
printExact(Math.pow(-1.0000001, Number.POSITIVE_INFINITY));

printExact(Math.pow(1.0000001, Number.NEGATIVE_INFINITY));
printExact(Math.pow(-1.0000001, Number.NEGATIVE_INFINITY));

printExact(Math.pow(1, Number.POSITIVE_INFINITY));
printExact(Math.pow(-1, Number.POSITIVE_INFINITY));
printExact(Math.pow(1, Number.NEGATIVE_INFINITY));
printExact(Math.pow(-1, Number.NEGATIVE_INFINITY));

printExact(Math.pow(0.999999, Number.POSITIVE_INFINITY));
printExact(Math.pow(-0.999999, Number.POSITIVE_INFINITY));
printExact(Math.pow(0.999999, Number.NEGATIVE_INFINITY));
printExact(Math.pow(-0.999999, Number.NEGATIVE_INFINITY));

printExact(Math.pow(Number.POSITIVE_INFINITY, 0.000001));
printExact(Math.pow(Number.POSITIVE_INFINITY, -0.000001));

printExact(Math.pow(Number.NEGATIVE_INFINITY, 3));  // odd integer
printExact(Math.pow(Number.NEGATIVE_INFINITY, 4));
printExact(Math.pow(Number.NEGATIVE_INFINITY, 4.5));
printExact(Math.pow(Number.NEGATIVE_INFINITY, Number.POSITIVE_INFINITY));

printExact(Math.pow(Number.NEGATIVE_INFINITY, -3));
printExact(Math.pow(Number.NEGATIVE_INFINITY, -4));
printExact(Math.pow(Number.NEGATIVE_INFINITY, -4.5));
printExact(Math.pow(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY));

/*===
pow 3
0
0
Infinity
Infinity
-0
0
0
0
-Infinity
Infinity
Infinity
Infinity
NaN
NaN
15213989025
-123345000
1000000
-8107
66
===*/

print('pow 3');

printExact(Math.pow(+0, 0.0000001));
printExact(Math.pow(+0, Number.POSITIVE_INFINITY));

printExact(Math.pow(+0, -0.000001));
printExact(Math.pow(+0, Number.NEGATIVE_INFINITY));

printExact(Math.pow(-0, 3));
printExact(Math.pow(-0, 4));
printExact(Math.pow(-0, 4.5));
printExact(Math.pow(-0, Number.POSITIVE_INFINITY));

printExact(Math.pow(-0, -3));
printExact(Math.pow(-0, -4));
printExact(Math.pow(-0, -4.5));
printExact(Math.pow(-0, Number.NEGATIVE_INFINITY));

// x < 0 and x is finite; y is finite and not an integer
printExact(Math.pow(-123.345, -123.1));
printExact(Math.pow(-123.345, 123.1));

// x < 0 and x is finite, y is finite and an integer
printRounded6(Math.pow(-123.345, 2));
printRounded6(Math.pow(-123.345, 1));
printRounded6(Math.pow(-123.345, 0));
printRounded6(Math.pow(-123.345, -1));
printRounded6(Math.pow(-123.345, -2));
