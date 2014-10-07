/*
 *  Math object (E5 Section 15.8).
 *
 *  Difficulties arise from:
 *
 *    - Trigonometric functions are not required to be bit exact
 *    - Random number testing
 *    - Some calls distinguish between +0 and -0 (e.g. asin)
 *
 *  To distinguish zero signs, the idiom '1 / x' is used.  If x is +0,
 *  this will result in Infinity, and if x is -0, it will result in
 *  -Infinity.
 */

function printRounded6(x) {
    print(Math.round(x * 1000000));
}

function printExact(x) {
    print(x.toString());
}

function zeroSign(x) {
    if (x !== 0) {
        return 'zero expected';
    }
    if (1 / x >= 0) {
        return 1;
    } else {
        return -1;
    }
}

var pr6 = printRounded6;
var prE = printExact;

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
pr6(Math.E);
pr6(Math.LN10);
pr6(Math.LN2);
pr6(Math.LOG2E);
pr6(Math.LOG10E);
pr6(Math.PI);
pr6(Math.SQRT1_2);
pr6(Math.SQRT2);

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
prE(Math.E);
prE(Math.LN10);
prE(Math.LN2);
prE(Math.LOG2E);
prE(Math.LOG10E);
prE(Math.PI);
prE(Math.SQRT1_2);
prE(Math.SQRT2);

/*===
abs
123
123
0
1
0
1
NaN
Infinity
Infinity
===*/

print('abs');
prE(Math.abs(+123));
prE(Math.abs(-123));
prE(Math.abs(+0));
print(zeroSign(Math.abs(+0)));
prE(Math.abs(-0));
print(zeroSign(Math.abs(-0)));
prE(Math.abs(Number.NaN));
prE(Math.abs(Number.POSITIVE_INFINITY));
prE(Math.abs(Number.NEGATIVE_INFINITY));

/*===
acos
1570796
0
1
3141593
NaN
NaN
NaN
===*/

print('acos');
pr6(Math.acos(0));
prE(Math.acos(1));
print(zeroSign(Math.acos(1)));
pr6(Math.acos(-1))
pr6(Math.acos(Number.NaN));
pr6(Math.acos(1.0000001));
pr6(Math.acos(-1.0000001));

/*===
asin
0
1
0
-1
1570796
-1570796
NaN
NaN
NaN
===*/

print('asin');
prE(Math.asin(+0));
print(zeroSign(Math.asin(+0)));
prE(Math.asin(-0));
print(zeroSign(Math.asin(-0)));
pr6(Math.asin(1));
pr6(Math.asin(-1))
pr6(Math.asin(Number.NaN));
pr6(Math.asin(1.0000001));
pr6(Math.asin(-1.0000001));

/*===
atan
NaN
0
1
0
-1
1570796
-1570796
===*/

print('atan');
prE(Math.atan(Number.NaN));
prE(Math.atan(+0));
print(zeroSign(Math.atan(+0)));
prE(Math.atan(-0));
print(zeroSign(Math.atan(-0)));
pr6(Math.atan(Number.POSITIVE_INFINITY));
pr6(Math.atan(Number.NEGATIVE_INFINITY));

/*===
atan2
NaN
NaN
NaN
1570796
1570796
0
1
0
1
3141593
3141593
0
-1
0
-1
-3141593
-3141593
-1570796
-1570796
0
1
3141593
0
-1
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

print('atan2');
prE(Math.atan2(NaN, NaN));
prE(Math.atan2(1, NaN));
prE(Math.atan2(NaN, 1));
pr6(Math.atan2(1, +0));
pr6(Math.atan2(1, -0));

prE(Math.atan2(+0, 1)); print(zeroSign(Math.atan2(+0, 1)));
prE(Math.atan2(+0, +0)); print(zeroSign(Math.atan2(+0, +0)));
pr6(Math.atan2(+0, -0));
pr6(Math.atan2(+0, -1));

prE(Math.atan2(-0, 1)); print(zeroSign(Math.atan2(-0, 1)));
prE(Math.atan2(-0, +0)); print(zeroSign(Math.atan2(-0, +0)));
pr6(Math.atan2(-0, -0));
pr6(Math.atan2(-0, -1));

pr6(Math.atan2(-1, +0));
pr6(Math.atan2(-1, -0));

prE(Math.atan2(1, Number.POSITIVE_INFINITY)); print(zeroSign(Math.atan2(1, Number.POSITIVE_INFINITY)));
pr6(Math.atan2(1, Number.NEGATIVE_INFINITY));
prE(Math.atan2(-1, Number.POSITIVE_INFINITY)); print(zeroSign(Math.atan2(-1, Number.POSITIVE_INFINITY)));
pr6(Math.atan2(-1, Number.NEGATIVE_INFINITY));

pr6(Math.atan2(Number.POSITIVE_INFINITY, 1));
pr6(Math.atan2(Number.POSITIVE_INFINITY, -1));
pr6(Math.atan2(Number.NEGATIVE_INFINITY, 1));
pr6(Math.atan2(Number.NEGATIVE_INFINITY, -1));

pr6(Math.atan2(Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY));
pr6(Math.atan2(Number.POSITIVE_INFINITY, Number.NEGATIVE_INFINITY));
pr6(Math.atan2(Number.NEGATIVE_INFINITY, Number.POSITIVE_INFINITY));
pr6(Math.atan2(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY));

/*===
ceil
123
124
-123
-122
4294967307
NaN
0
1
0
-1
Infinity
-Infinity
0
-1
-1
===*/

print('ceil');

prE(Math.ceil(123));
prE(Math.ceil(123.000001));
prE(Math.ceil(-123));
prE(Math.ceil(-122.999999));

prE(Math.ceil(4294967306.5));  // value higher than 32 bits

prE(Math.ceil(NaN));
prE(Math.ceil(+0)); print(zeroSign(Math.ceil(+0)));
prE(Math.ceil(-0)); print(zeroSign(Math.ceil(-0)));
prE(Number.POSITIVE_INFINITY);
prE(Number.NEGATIVE_INFINITY);
prE(Math.ceil(-0.5)); print(zeroSign(Math.ceil(-0.5)));
prE(Math.ceil(-1.0));

/*===
cos
NaN
1
1
NaN
NaN
540302
===*/

print('cos');

prE(Math.cos(NaN));
prE(Math.cos(+0));
prE(Math.cos(-0));
prE(Math.cos(Number.POSITIVE_INFINITY));
prE(Math.cos(Number.NEGATIVE_INFINITY));
pr6(Math.cos(1));

/*===
exp
NaN
1
1
Infinity
0
1
===*/

print('exp');

prE(Math.exp(NaN));
prE(Math.exp(+0));
prE(Math.exp(-0));
prE(Math.exp(Number.POSITIVE_INFINITY));
prE(Math.exp(Number.NEGATIVE_INFINITY)); print(zeroSign(Math.exp(Number.NEGATIVE_INFINITY)));

/*===
floor
123
122
-123
-124
4294967306
NaN
0
1
0
-1
Infinity
-Infinity
0
1
1
===*/

print('floor');

prE(Math.floor(123));
prE(Math.floor(122.999999));
prE(Math.floor(-123));
prE(Math.floor(-124));

prE(Math.floor(4294967306.5));  // value higher than 32 bits

prE(Math.floor(NaN));
prE(Math.floor(+0)); print(zeroSign(Math.floor(+0)));
prE(Math.floor(-0)); print(zeroSign(Math.floor(-0)));
prE(Math.floor(Number.POSITIVE_INFINITY));
prE(Math.floor(Number.NEGATIVE_INFINITY));
prE(Math.floor(0.5)); print(zeroSign(Math.floor(0.5)));
prE(Math.floor(1));

/*===
log
4812184
NaN
NaN
NaN
-Infinity
-Infinity
0
1
Infinity
===*/

print('log');

pr6(Math.log(123));

prE(Math.log(NaN));

prE(Math.log(-0.000001));
prE(Math.log(Number.NEGATIVE_INFINITY));

prE(Math.log(+0));
prE(Math.log(-0));

prE(Math.log(1)); print(zeroSign(Math.log(1)));

prE(Math.log(Number.POSITIVE_INFINITY));

/*===
max
456
Infinity
123
0
1
0
-1
-Infinity
NaN
0
1
0
1
===*/

print('max');

prE(Math.max(123, -123, 456, 234));
prE(Math.max(123, -123, Number.POSITIVE_INFINITY, 234));
prE(Math.max(123));

prE(Math.max(+0));
print(zeroSign(Math.max(+0)));
prE(Math.max(-0));
print(zeroSign(Math.max(-0)));

prE(Math.max());  // -> -Infinity (!)
prE(Math.max(1,2,NaN,4));

prE(Math.max(+0, -0));  // -> +0
print(zeroSign(Math.max(+0, -0)));

prE(Math.max(-0, +0));  // -> +0
print(zeroSign(Math.max(-0, +0)));

/*===
min
-123
-Infinity
123
0
1
0
-1
Infinity
NaN
0
-1
0
-1
===*/

print('min');

prE(Math.min(123, -123, 456, 234));
prE(Math.min(123, -123, Number.NEGATIVE_INFINITY, 234));
prE(Math.min(123));

prE(Math.min(+0));
print(zeroSign(Math.min(+0)));
prE(Math.min(-0));
print(zeroSign(Math.min(-0)));

prE(Math.min());  // -> Infinity (!)
prE(Math.min(1,2,NaN,4));

prE(Math.min(+0, -0));  // -> -0
print(zeroSign(Math.min(+0, -0)));

prE(Math.min(-0, +0));  // -> -0
print(zeroSign(Math.min(-0, +0)));

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

prE(Math.pow(Number.NEGATIVE_INFINITY, NaN));
prE(Math.pow(-1, NaN));
prE(Math.pow(-0, NaN));
prE(Math.pow(+0, NaN));
prE(Math.pow(1, NaN));
prE(Math.pow(Number.POSITIVE_INFINITY, NaN));

prE(Math.pow(Number.NEGATIVE_INFINITY, +0));
prE(Math.pow(-1, +0));
prE(Math.pow(-0, +0));
prE(Math.pow(+0, +0));
prE(Math.pow(1, +0));
prE(Math.pow(Number.POSITIVE_INFINITY, +0));
prE(Math.pow(NaN, +0));

prE(Math.pow(Number.NEGATIVE_INFINITY, -0));
prE(Math.pow(-1, -0));
prE(Math.pow(-0, -0));
prE(Math.pow(+0, -0));
prE(Math.pow(1, -0));
prE(Math.pow(Number.POSITIVE_INFINITY, -0));
prE(Math.pow(NaN, -0));

prE(Math.pow(NaN, Number.NEGATIVE_INFINITY));
prE(Math.pow(NaN, -1));
prE(Math.pow(NaN, +1));
prE(Math.pow(NaN, Number.POSITIVE_INFINITY));

/*===
pow 2
Infinity
Infinity
0
1
0
1
NaN
NaN
NaN
NaN
0
1
0
1
Infinity
Infinity
Infinity
0
1
-Infinity
Infinity
Infinity
Infinity
0
-1
0
1
0
1
0
1
===*/

print('pow 2');

prE(Math.pow(1.0000001, Number.POSITIVE_INFINITY));
prE(Math.pow(-1.0000001, Number.POSITIVE_INFINITY));

prE(Math.pow(1.0000001, Number.NEGATIVE_INFINITY));
print(zeroSign(Math.pow(1.0000001, Number.NEGATIVE_INFINITY)));
prE(Math.pow(-1.0000001, Number.NEGATIVE_INFINITY));
print(zeroSign(Math.pow(-1.0000001, Number.NEGATIVE_INFINITY)));

prE(Math.pow(1, Number.POSITIVE_INFINITY));
prE(Math.pow(-1, Number.POSITIVE_INFINITY));
prE(Math.pow(1, Number.NEGATIVE_INFINITY));
prE(Math.pow(-1, Number.NEGATIVE_INFINITY));

prE(Math.pow(0.999999, Number.POSITIVE_INFINITY));
print(zeroSign(Math.pow(0.999999, Number.POSITIVE_INFINITY)));
prE(Math.pow(-0.999999, Number.POSITIVE_INFINITY));
print(zeroSign(Math.pow(-0.999999, Number.POSITIVE_INFINITY)));
prE(Math.pow(0.999999, Number.NEGATIVE_INFINITY));
prE(Math.pow(-0.999999, Number.NEGATIVE_INFINITY));

prE(Math.pow(Number.POSITIVE_INFINITY, 0.000001));
prE(Math.pow(Number.POSITIVE_INFINITY, -0.000001));
print(zeroSign(Math.pow(Number.POSITIVE_INFINITY, -0.000001)));

prE(Math.pow(Number.NEGATIVE_INFINITY, 3));  // odd integer
prE(Math.pow(Number.NEGATIVE_INFINITY, 4));
prE(Math.pow(Number.NEGATIVE_INFINITY, 4.5));
prE(Math.pow(Number.NEGATIVE_INFINITY, Number.POSITIVE_INFINITY));

prE(Math.pow(Number.NEGATIVE_INFINITY, -3));
print(zeroSign(Math.pow(Number.NEGATIVE_INFINITY, -3)));
prE(Math.pow(Number.NEGATIVE_INFINITY, -4));
print(zeroSign(Math.pow(Number.NEGATIVE_INFINITY, -4)));
prE(Math.pow(Number.NEGATIVE_INFINITY, -4.5));
print(zeroSign(Math.pow(Number.NEGATIVE_INFINITY, -4.5)));
prE(Math.pow(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY));
print(zeroSign(Math.pow(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY)));

/*===
pow 3
0
1
0
1
Infinity
Infinity
0
-1
0
1
0
1
0
1
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

prE(Math.pow(+0, 0.0000001));
print(zeroSign(Math.pow(+0, 0.0000001)));
prE(Math.pow(+0, Number.POSITIVE_INFINITY));
print(zeroSign(Math.pow(+0, Number.POSITIVE_INFINITY)));

prE(Math.pow(+0, -0.000001));
prE(Math.pow(+0, Number.NEGATIVE_INFINITY));

prE(Math.pow(-0, 3));
print(zeroSign(Math.pow(-0, 3)));
prE(Math.pow(-0, 4));
print(zeroSign(Math.pow(-0, 4)));
prE(Math.pow(-0, 4.5));
print(zeroSign(Math.pow(-0, 4.5)));
prE(Math.pow(-0, Number.POSITIVE_INFINITY));
print(zeroSign(Math.pow(-0, Number.POSITIVE_INFINITY)));

prE(Math.pow(-0, -3));
prE(Math.pow(-0, -4));
prE(Math.pow(-0, -4.5));
prE(Math.pow(-0, Number.NEGATIVE_INFINITY));

// x < 0 and x is finite; y is finite and not an integer
prE(Math.pow(-123.345, -123.1));
prE(Math.pow(-123.345, 123.1));

// x < 0 and x is finite, y is finite and an integer
pr6(Math.pow(-123.345, 2));
pr6(Math.pow(-123.345, 1));
pr6(Math.pow(-123.345, 0));
pr6(Math.pow(-123.345, -1));
pr6(Math.pow(-123.345, -2));

/*===
random
true true
true true
===*/

/* This is a statistical test, but should pass almost always. */

print('random');

var rnd_val;
var rnd_sum = 0;
var rnd_ge0 = true;
var rnd_lt0 = true;
for (i = 0; i < 100000; i++) {
    rnd_val = Math.random();
    if (!(rnd_val >= 0)) { rnd_ge0 = false; }
    if (!(rnd_val < 1)) { rnd_lt0 = false; }
    rnd_sum += rnd_val;
}

print(rnd_ge0, rnd_lt0);
print(rnd_sum >= 49700.0, rnd_sum <= 50300.0);

/*===
round
NaN
0
1
0
-1
Infinity
-Infinity
0
1
1
0
-1
-1
4
-3
===*/

print('round');

prE(Math.round(NaN));
prE(Math.round(+0));
print(zeroSign(Math.round(+0)));
prE(Math.round(-0));
print(zeroSign(Math.round(-0)));
prE(Math.round(Number.POSITIVE_INFINITY));
prE(Math.round(Number.NEGATIVE_INFINITY));
prE(Math.round(0.400009));
print(zeroSign(Math.round(0.400009)));
prE(Math.round(0.5));
prE(Math.round(-0.5));
print(zeroSign(Math.round(-0.5)));
print(Math.round(-0.500001));
prE(Math.round(3.5));   // tie break towards +Infinity -> 4
prE(Math.round(-3.5));  // tie break towards +Infinity -> -3

/*===
sin
NaN
0
1
0
-1
NaN
NaN
841471
===*/

print('sin');

prE(Math.sin(NaN));
prE(Math.sin(+0));
print(zeroSign(Math.sin(+0)));
prE(Math.sin(-0));
print(zeroSign(Math.sin(-0)));
prE(Math.sin(Number.POSITIVE_INFINITY));
prE(Math.sin(Number.NEGATIVE_INFINITY));
pr6(Math.sin(1));

/*===
sqrt
2000000
1414214
NaN
NaN
NaN
0
1
0
-1
Infinity
===*/

print('sqrt');

pr6(Math.sqrt(4));
pr6(Math.sqrt(2));

prE(Math.sqrt(NaN));
prE(Math.sqrt(-0.000001));
prE(Math.sqrt(Number.NEGATIVE_INFINITY));
prE(Math.sqrt(+0));
print(zeroSign(Math.sqrt(+0)));
prE(Math.sqrt(-0));
print(zeroSign(Math.sqrt(-0)));
prE(Math.sqrt(Number.POSITIVE_INFINITY));

/*===
tan
1557408
-2185040
648361
NaN
0
1
0
-1
NaN
NaN
===*/

print('tan');

pr6(Math.tan(1));
pr6(Math.tan(2));
pr6(Math.tan(10));

prE(Math.tan(NaN));
prE(Math.tan(+0));
print(zeroSign(Math.tan(+0)));
prE(Math.tan(-0));
print(zeroSign(Math.tan(-0)));
prE(Math.tan(Number.POSITIVE_INFINITY));
prE(Math.tan(Number.NEGATIVE_INFINITY));
