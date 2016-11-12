/*
 *  Math constants
 */

/*@include util-number.js@*/

/*===
rounded constant test
2718282
2302585
693147
1442695
434294
3141593
707107
1414214
===*/

/* Rounded constant test. */

print('rounded constant test');
printRounded6(Math.E);
printRounded6(Math.LN10);
printRounded6(Math.LN2);
printRounded6(Math.LOG2E);
printRounded6(Math.LOG10E);
printRounded6(Math.PI);
printRounded6(Math.SQRT1_2);
printRounded6(Math.SQRT2);

/*===
exact constant test
2.718281828459045
2.302585092994046
0.6931471805599453
1.4426950408889634
0.4342944819032518
3.141592653589793
0.7071067811865476
1.4142135623730951
===*/

/* Exact constant test. The results were taken from V8 and compared
 * against Rhino.
 */

print('exact constant test');
printExact(Math.E);
printExact(Math.LN10);
printExact(Math.LN2);
printExact(Math.LOG2E);
printExact(Math.LOG10E);
printExact(Math.PI);
printExact(Math.SQRT1_2);
printExact(Math.SQRT2);
