/*
 *  Tests for E5 Section 11.8.5.
 */

function f(x,y) {
    print(x < y);
}

/*===
false
false
true
false
false
true
false
false
true
===*/

f('aax', 'aaw');
f('aax', 'aax');
f('aax', 'aay');

f('aa' + String.fromCharCode(200), 'aa' + String.fromCharCode(199));
f('aa' + String.fromCharCode(200), 'aa' + String.fromCharCode(200));
f('aa' + String.fromCharCode(200), 'aa' + String.fromCharCode(201));

f('aa' + String.fromCharCode(200, 3000), 'aa' + String.fromCharCode(200, 2999));
f('aa' + String.fromCharCode(200, 3000), 'aa' + String.fromCharCode(200, 3000));
f('aa' + String.fromCharCode(200, 3000), 'aa' + String.fromCharCode(200, 3001));

/*===
false
false
true
false
===*/

/* String cases, prefix matches -> length matters. */

f('', '');
f('xx', 'xx');
f('xx', 'xxx');
f('xx', 'x');

/* XXX: more string cases, esp. unicode */

/*===
false
false
false
false
false
false
false
false
false
false
false
false
false
===*/

/* Numeric cases involving NaN (steps c and d) */

f(NaN, Number.NEGATIVE_INFINITY);
f(NaN, -1);
f(NaN, -0);
f(NaN, +0);
f(NaN, 1);
f(NaN, Number.POSITIVE_INFINITY);

f(Number.NEGATIVE_INFINITY, NaN);
f(-1, NaN);
f(-0, NaN);
f(+0, NaN);
f(1, NaN);
f(Number.POSITIVE_INFINITY, NaN);

f(NaN, NaN);

/*===
false
false
false
false
false
false
===*/

/* Cases involving same number value (step e) */

f(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY);
f(-1, -1);
f(-0, -0);
f(+0, +0);
f(1, 1);
f(Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY);

/*===
false
false
false
false
===*/

/* Zeroes always compare to false: same sign cases are matched by step e,
 * different sign cases by steps f and g.
 */

f(-0, -0);
f(-0, +0);
f(+0, -0);
f(+0, +0);

/*===
false
false
false
false
false
false
true
true
true
true
true
false
false
false
false
false
false
false
false
true
true
true
true
true
===*/

/* Infinity cases */

// step h
f(Number.POSITIVE_INFINITY, Number.NEGATIVE_INFINITY);
f(Number.POSITIVE_INFINITY, -1);
f(Number.POSITIVE_INFINITY, -0);
f(Number.POSITIVE_INFINITY, +0);
f(Number.POSITIVE_INFINITY, 1);
f(Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY);  // actually covered by step e -> false

// step i
f(Number.NEGATIVE_INFINITY, Number.POSITIVE_INFINITY);
f(-1, Number.POSITIVE_INFINITY);
f(-0, Number.POSITIVE_INFINITY);
f(+0, Number.POSITIVE_INFINITY);
f(1, Number.POSITIVE_INFINITY);
f(Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY);  // actually covered by step e -> false

// step j
f(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY);  // actually covered by step e -> false
f(-1, Number.NEGATIVE_INFINITY);
f(-0, Number.NEGATIVE_INFINITY);
f(+0, Number.NEGATIVE_INFINITY);
f(1, Number.NEGATIVE_INFINITY);
f(Number.POSITIVE_INFINITY, Number.NEGATIVE_INFINITY);

// step k
f(Number.NEGATIVE_INFINITY, Number.NEGATIVE_INFINITY);  // actually covered by step e -> false
f(Number.NEGATIVE_INFINITY, -1);
f(Number.NEGATIVE_INFINITY, -0);
f(Number.NEGATIVE_INFINITY, +0);
f(Number.NEGATIVE_INFINITY, 1);
f(Number.NEGATIVE_INFINITY, Number.POSITIVE_INFINITY);

/*===
true
false
===*/

/* Normal number cases */

f(-1, 1);
f(1, -1);
