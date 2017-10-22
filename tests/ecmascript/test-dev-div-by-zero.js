/*
 *  Floating point division by zero is undefined behavior (in C99+)
 *  so the internal implementation must work around it by implementing
 *  the division manually for portability.
 *
 *  Exercise a few cases in compiler/executor.
 */

/*===
0.5 1 -1 NaN 0 0
compiler
Infinity
-Infinity
-Infinity
Infinity
NaN
NaN
executor
Infinity
-Infinity
-Infinity
Infinity
NaN
NaN
===*/

// Weird arithmetic to ensure values are not fastints.
var half = 0.5;
var one = 1; one += half; one -= half;
var mone = -1; mone += half; mone -= half;
var nan = NaN;
var zero = 0; zero += half; zero -= half;
var mzero = -0;
print(half, one, mone, nan, zero, mzero);

// Compiler handles.
print('compiler');
print((1.5 - 0.5) / (0.5 - 0.5));
print((1.5 - 0.5) / -0);
print((-1.5 + 0.5) / (0.5 - 0.5));
print((-1.5 + 0.5) / -0);
print(nan / (0.5 - 0.5));
print(nan / -0);

// Executor handles.
print('executor');
print(one / zero);
print(one / mzero);
print(mone / zero);
print(mone / mzero);
print(nan / zero);
print(nan / mzero);
