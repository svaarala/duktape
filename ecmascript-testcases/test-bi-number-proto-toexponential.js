/*
 *  toExponential()
 *
 *  NOTE: The algorithm for toExponential() in E5.1 Section 15.7.4.6 handles
 *  NaN and +/-Infinity as a special case *before* the digit count is checked.
 *  So, for NaN and infinities, x.toExponential() never throws a RangeError
 *  for incorrect fraction digit count like it does for other numbers.  V8
 *  behavior differs from this: it will throw a RangeError.  Rhino behaves
 *  as the specification requires but has other issues (it won't validate
 *  fractionDigits).
 *
 *  Test case expect strings are mostly based on V8, but with clear V8
 *  issues fixed manually.
 */

function test(this_value, args, prefix_string) {
    var t;

    if (prefix_string === undefined) {
        prefix_string = '';
    } else {
        prefix_string += ' ';
    }

    try {
        var t = Number.prototype.toExponential.apply(this_value, args);
        print(prefix_string + typeof t, t);
    } catch (e) {
        print(e.name);
    }
}

/*===
basic
string NaN
string -Infinity
string Infinity
0,undefined string -Infinity
0,notgiven string -Infinity
0,-1 string -Infinity
0,0 string -Infinity
0,1 string -Infinity
0,2 string -Infinity
0,3 string -Infinity
0,4 string -Infinity
0,5 string -Infinity
0,6 string -Infinity
0,7 string -Infinity
0,8 string -Infinity
0,9 string -Infinity
0,10 string -Infinity
0,11 string -Infinity
0,12 string -Infinity
0,13 string -Infinity
0,14 string -Infinity
0,15 string -Infinity
0,16 string -Infinity
0,17 string -Infinity
0,18 string -Infinity
0,19 string -Infinity
0,20 string -Infinity
0,21 string -Infinity
1,undefined string -1e+100
1,notgiven string -1e+100
RangeError
1,0 string -1e+100
1,1 string -1.0e+100
1,2 string -1.00e+100
1,3 string -1.000e+100
1,4 string -1.0000e+100
1,5 string -1.00000e+100
1,6 string -1.000000e+100
1,7 string -1.0000000e+100
1,8 string -1.00000000e+100
1,9 string -1.000000000e+100
1,10 string -1.0000000000e+100
1,11 string -1.00000000000e+100
1,12 string -1.000000000000e+100
1,13 string -1.0000000000000e+100
1,14 string -1.00000000000000e+100
1,15 string -1.000000000000000e+100
1,16 string -1.0000000000000000e+100
1,17 string -1.00000000000000002e+100
1,18 string -1.000000000000000016e+100
1,19 string -1.0000000000000000159e+100
1,20 string -1.00000000000000001590e+100
RangeError
2,undefined string -1e+21
2,notgiven string -1e+21
RangeError
2,0 string -1e+21
2,1 string -1.0e+21
2,2 string -1.00e+21
2,3 string -1.000e+21
2,4 string -1.0000e+21
2,5 string -1.00000e+21
2,6 string -1.000000e+21
2,7 string -1.0000000e+21
2,8 string -1.00000000e+21
2,9 string -1.000000000e+21
2,10 string -1.0000000000e+21
2,11 string -1.00000000000e+21
2,12 string -1.000000000000e+21
2,13 string -1.0000000000000e+21
2,14 string -1.00000000000000e+21
2,15 string -1.000000000000000e+21
2,16 string -1.0000000000000000e+21
2,17 string -1.00000000000000000e+21
2,18 string -1.000000000000000000e+21
2,19 string -1.0000000000000000000e+21
2,20 string -1.00000000000000000000e+21
RangeError
3,undefined string -9.8765e+20
3,notgiven string -9.8765e+20
RangeError
3,0 string -1e+21
3,1 string -9.9e+20
3,2 string -9.88e+20
3,3 string -9.876e+20
3,4 string -9.8765e+20
3,5 string -9.87650e+20
3,6 string -9.876500e+20
3,7 string -9.8765000e+20
3,8 string -9.87650000e+20
3,9 string -9.876500000e+20
3,10 string -9.8765000000e+20
3,11 string -9.87650000000e+20
3,12 string -9.876500000000e+20
3,13 string -9.8765000000000e+20
3,14 string -9.87650000000000e+20
3,15 string -9.876499999999999e+20
3,16 string -9.8764999999999993e+20
3,17 string -9.87649999999999934e+20
3,18 string -9.876499999999999345e+20
3,19 string -9.8764999999999993446e+20
3,20 string -9.87649999999999934464e+20
RangeError
4,undefined string -1.2345678901234567e+4
4,notgiven string -1.2345678901234567e+4
RangeError
4,0 string -1e+4
4,1 string -1.2e+4
4,2 string -1.23e+4
4,3 string -1.235e+4
4,4 string -1.2346e+4
4,5 string -1.23457e+4
4,6 string -1.234568e+4
4,7 string -1.2345679e+4
4,8 string -1.23456789e+4
4,9 string -1.234567890e+4
4,10 string -1.2345678901e+4
4,11 string -1.23456789012e+4
4,12 string -1.234567890123e+4
4,13 string -1.2345678901235e+4
4,14 string -1.23456789012346e+4
4,15 string -1.234567890123457e+4
4,16 string -1.2345678901234567e+4
4,17 string -1.23456789012345671e+4
4,18 string -1.234567890123456709e+4
4,19 string -1.2345678901234567093e+4
4,20 string -1.23456789012345670926e+4
RangeError
5,undefined string -1.2345678901234568e-1
5,notgiven string -1.2345678901234568e-1
RangeError
5,0 string -1e-1
5,1 string -1.2e-1
5,2 string -1.23e-1
5,3 string -1.235e-1
5,4 string -1.2346e-1
5,5 string -1.23457e-1
5,6 string -1.234568e-1
5,7 string -1.2345679e-1
5,8 string -1.23456789e-1
5,9 string -1.234567890e-1
5,10 string -1.2345678901e-1
5,11 string -1.23456789012e-1
5,12 string -1.234567890123e-1
5,13 string -1.2345678901235e-1
5,14 string -1.23456789012346e-1
5,15 string -1.234567890123457e-1
5,16 string -1.2345678901234568e-1
5,17 string -1.23456789012345677e-1
5,18 string -1.234567890123456774e-1
5,19 string -1.2345678901234567737e-1
5,20 string -1.23456789012345677370e-1
RangeError
6,undefined string -1.2345678901234568e-21
6,notgiven string -1.2345678901234568e-21
RangeError
6,0 string -1e-21
6,1 string -1.2e-21
6,2 string -1.23e-21
6,3 string -1.235e-21
6,4 string -1.2346e-21
6,5 string -1.23457e-21
6,6 string -1.234568e-21
6,7 string -1.2345679e-21
6,8 string -1.23456789e-21
6,9 string -1.234567890e-21
6,10 string -1.2345678901e-21
6,11 string -1.23456789012e-21
6,12 string -1.234567890123e-21
6,13 string -1.2345678901235e-21
6,14 string -1.23456789012346e-21
6,15 string -1.234567890123457e-21
6,16 string -1.2345678901234568e-21
6,17 string -1.23456789012345682e-21
6,18 string -1.234567890123456824e-21
6,19 string -1.2345678901234568242e-21
6,20 string -1.23456789012345682417e-21
RangeError
7,undefined string -1.2345678901234568e-121
7,notgiven string -1.2345678901234568e-121
RangeError
7,0 string -1e-121
7,1 string -1.2e-121
7,2 string -1.23e-121
7,3 string -1.235e-121
7,4 string -1.2346e-121
7,5 string -1.23457e-121
7,6 string -1.234568e-121
7,7 string -1.2345679e-121
7,8 string -1.23456789e-121
7,9 string -1.234567890e-121
7,10 string -1.2345678901e-121
7,11 string -1.23456789012e-121
7,12 string -1.234567890123e-121
7,13 string -1.2345678901235e-121
7,14 string -1.23456789012346e-121
7,15 string -1.234567890123457e-121
7,16 string -1.2345678901234568e-121
7,17 string -1.23456789012345684e-121
7,18 string -1.234567890123456835e-121
7,19 string -1.2345678901234568352e-121
7,20 string -1.23456789012345683522e-121
RangeError
8,undefined string 0e+0
8,notgiven string 0e+0
RangeError
8,0 string 0e+0
8,1 string 0.0e+0
8,2 string 0.00e+0
8,3 string 0.000e+0
8,4 string 0.0000e+0
8,5 string 0.00000e+0
8,6 string 0.000000e+0
8,7 string 0.0000000e+0
8,8 string 0.00000000e+0
8,9 string 0.000000000e+0
8,10 string 0.0000000000e+0
8,11 string 0.00000000000e+0
8,12 string 0.000000000000e+0
8,13 string 0.0000000000000e+0
8,14 string 0.00000000000000e+0
8,15 string 0.000000000000000e+0
8,16 string 0.0000000000000000e+0
8,17 string 0.00000000000000000e+0
8,18 string 0.000000000000000000e+0
8,19 string 0.0000000000000000000e+0
8,20 string 0.00000000000000000000e+0
RangeError
9,undefined string 0e+0
9,notgiven string 0e+0
RangeError
9,0 string 0e+0
9,1 string 0.0e+0
9,2 string 0.00e+0
9,3 string 0.000e+0
9,4 string 0.0000e+0
9,5 string 0.00000e+0
9,6 string 0.000000e+0
9,7 string 0.0000000e+0
9,8 string 0.00000000e+0
9,9 string 0.000000000e+0
9,10 string 0.0000000000e+0
9,11 string 0.00000000000e+0
9,12 string 0.000000000000e+0
9,13 string 0.0000000000000e+0
9,14 string 0.00000000000000e+0
9,15 string 0.000000000000000e+0
9,16 string 0.0000000000000000e+0
9,17 string 0.00000000000000000e+0
9,18 string 0.000000000000000000e+0
9,19 string 0.0000000000000000000e+0
9,20 string 0.00000000000000000000e+0
RangeError
10,undefined string 1.2345678901234568e-121
10,notgiven string 1.2345678901234568e-121
RangeError
10,0 string 1e-121
10,1 string 1.2e-121
10,2 string 1.23e-121
10,3 string 1.235e-121
10,4 string 1.2346e-121
10,5 string 1.23457e-121
10,6 string 1.234568e-121
10,7 string 1.2345679e-121
10,8 string 1.23456789e-121
10,9 string 1.234567890e-121
10,10 string 1.2345678901e-121
10,11 string 1.23456789012e-121
10,12 string 1.234567890123e-121
10,13 string 1.2345678901235e-121
10,14 string 1.23456789012346e-121
10,15 string 1.234567890123457e-121
10,16 string 1.2345678901234568e-121
10,17 string 1.23456789012345684e-121
10,18 string 1.234567890123456835e-121
10,19 string 1.2345678901234568352e-121
10,20 string 1.23456789012345683522e-121
RangeError
11,undefined string 1.2345678901234568e-21
11,notgiven string 1.2345678901234568e-21
RangeError
11,0 string 1e-21
11,1 string 1.2e-21
11,2 string 1.23e-21
11,3 string 1.235e-21
11,4 string 1.2346e-21
11,5 string 1.23457e-21
11,6 string 1.234568e-21
11,7 string 1.2345679e-21
11,8 string 1.23456789e-21
11,9 string 1.234567890e-21
11,10 string 1.2345678901e-21
11,11 string 1.23456789012e-21
11,12 string 1.234567890123e-21
11,13 string 1.2345678901235e-21
11,14 string 1.23456789012346e-21
11,15 string 1.234567890123457e-21
11,16 string 1.2345678901234568e-21
11,17 string 1.23456789012345682e-21
11,18 string 1.234567890123456824e-21
11,19 string 1.2345678901234568242e-21
11,20 string 1.23456789012345682417e-21
RangeError
12,undefined string 1.2345678901234568e-1
12,notgiven string 1.2345678901234568e-1
RangeError
12,0 string 1e-1
12,1 string 1.2e-1
12,2 string 1.23e-1
12,3 string 1.235e-1
12,4 string 1.2346e-1
12,5 string 1.23457e-1
12,6 string 1.234568e-1
12,7 string 1.2345679e-1
12,8 string 1.23456789e-1
12,9 string 1.234567890e-1
12,10 string 1.2345678901e-1
12,11 string 1.23456789012e-1
12,12 string 1.234567890123e-1
12,13 string 1.2345678901235e-1
12,14 string 1.23456789012346e-1
12,15 string 1.234567890123457e-1
12,16 string 1.2345678901234568e-1
12,17 string 1.23456789012345677e-1
12,18 string 1.234567890123456774e-1
12,19 string 1.2345678901234567737e-1
12,20 string 1.23456789012345677370e-1
RangeError
13,undefined string 1.2345678901234567e+4
13,notgiven string 1.2345678901234567e+4
RangeError
13,0 string 1e+4
13,1 string 1.2e+4
13,2 string 1.23e+4
13,3 string 1.235e+4
13,4 string 1.2346e+4
13,5 string 1.23457e+4
13,6 string 1.234568e+4
13,7 string 1.2345679e+4
13,8 string 1.23456789e+4
13,9 string 1.234567890e+4
13,10 string 1.2345678901e+4
13,11 string 1.23456789012e+4
13,12 string 1.234567890123e+4
13,13 string 1.2345678901235e+4
13,14 string 1.23456789012346e+4
13,15 string 1.234567890123457e+4
13,16 string 1.2345678901234567e+4
13,17 string 1.23456789012345671e+4
13,18 string 1.234567890123456709e+4
13,19 string 1.2345678901234567093e+4
13,20 string 1.23456789012345670926e+4
RangeError
14,undefined string 9.8765e+20
14,notgiven string 9.8765e+20
RangeError
14,0 string 1e+21
14,1 string 9.9e+20
14,2 string 9.88e+20
14,3 string 9.876e+20
14,4 string 9.8765e+20
14,5 string 9.87650e+20
14,6 string 9.876500e+20
14,7 string 9.8765000e+20
14,8 string 9.87650000e+20
14,9 string 9.876500000e+20
14,10 string 9.8765000000e+20
14,11 string 9.87650000000e+20
14,12 string 9.876500000000e+20
14,13 string 9.8765000000000e+20
14,14 string 9.87650000000000e+20
14,15 string 9.876499999999999e+20
14,16 string 9.8764999999999993e+20
14,17 string 9.87649999999999934e+20
14,18 string 9.876499999999999345e+20
14,19 string 9.8764999999999993446e+20
14,20 string 9.87649999999999934464e+20
RangeError
15,undefined string 1e+21
15,notgiven string 1e+21
RangeError
15,0 string 1e+21
15,1 string 1.0e+21
15,2 string 1.00e+21
15,3 string 1.000e+21
15,4 string 1.0000e+21
15,5 string 1.00000e+21
15,6 string 1.000000e+21
15,7 string 1.0000000e+21
15,8 string 1.00000000e+21
15,9 string 1.000000000e+21
15,10 string 1.0000000000e+21
15,11 string 1.00000000000e+21
15,12 string 1.000000000000e+21
15,13 string 1.0000000000000e+21
15,14 string 1.00000000000000e+21
15,15 string 1.000000000000000e+21
15,16 string 1.0000000000000000e+21
15,17 string 1.00000000000000000e+21
15,18 string 1.000000000000000000e+21
15,19 string 1.0000000000000000000e+21
15,20 string 1.00000000000000000000e+21
RangeError
16,undefined string -1e+100
16,notgiven string -1e+100
RangeError
16,0 string -1e+100
16,1 string -1.0e+100
16,2 string -1.00e+100
16,3 string -1.000e+100
16,4 string -1.0000e+100
16,5 string -1.00000e+100
16,6 string -1.000000e+100
16,7 string -1.0000000e+100
16,8 string -1.00000000e+100
16,9 string -1.000000000e+100
16,10 string -1.0000000000e+100
16,11 string -1.00000000000e+100
16,12 string -1.000000000000e+100
16,13 string -1.0000000000000e+100
16,14 string -1.00000000000000e+100
16,15 string -1.000000000000000e+100
16,16 string -1.0000000000000000e+100
16,17 string -1.00000000000000002e+100
16,18 string -1.000000000000000016e+100
16,19 string -1.0000000000000000159e+100
16,20 string -1.00000000000000001590e+100
RangeError
17,undefined string Infinity
17,notgiven string Infinity
17,-1 string Infinity
17,0 string Infinity
17,1 string Infinity
17,2 string Infinity
17,3 string Infinity
17,4 string Infinity
17,5 string Infinity
17,6 string Infinity
17,7 string Infinity
17,8 string Infinity
17,9 string Infinity
17,10 string Infinity
17,11 string Infinity
17,12 string Infinity
17,13 string Infinity
17,14 string Infinity
17,15 string Infinity
17,16 string Infinity
17,17 string Infinity
17,18 string Infinity
17,19 string Infinity
17,20 string Infinity
17,21 string Infinity
18,undefined string NaN
18,notgiven string NaN
18,-1 string NaN
18,0 string NaN
18,1 string NaN
18,2 string NaN
18,3 string NaN
18,4 string NaN
18,5 string NaN
18,6 string NaN
18,7 string NaN
18,8 string NaN
18,9 string NaN
18,10 string NaN
18,11 string NaN
18,12 string NaN
18,13 string NaN
18,14 string NaN
18,15 string NaN
18,16 string NaN
18,17 string NaN
18,18 string NaN
18,19 string NaN
18,20 string NaN
18,21 string NaN
===*/

print('basic');

function basicTest() {
    var values = [
        Number.NEGATIVE_INFINITY,
        -1e100,
        -1e21,
        -0.98765e21,
        -12345.6789012345678901234567890,
        -0.123456789012345678901234567890,
        -0.123456789012345678901234567890e-20,
        -0.123456789012345678901234567890e-120,
        -0,
        +0,
        0.123456789012345678901234567890e-120,
        0.123456789012345678901234567890e-20,
        0.123456789012345678901234567890,
        12345.6789012345678901234567890,
        0.98765e21,
        1e21,
        -1e100,
        Number.POSITIVE_INFINITY,
        Number.NaN
    ];

    // NaN and infinities are special cases; they are checked for *after*
    // ToInteger(fractionDigits) but *before* checking the fractionDigits
    // range.  V8 will throw a RangeError from these which seems incorrect.

    test(new Number(Number.NaN), [ 100 ]);
    test(new Number(Number.NEGATIVE_INFINITY), [ 100 ]);
    test(new Number(Number.POSITIVE_INFINITY), [ 100 ]);

    // test a bunch of value and fractionDigits combinations

    for (i = 0; i < values.length; i++) {
        // undefined fraction digits is a special case: use shortest digit count
        test(new Number(values[i]), [ undefined ], i + ',undefined');

        // not-given fraction digits should behave like undefined
        test(new Number(values[i]), [ undefined ], i + ',notgiven');

        for (f = -1; f <= 21; f++) {
            test(new Number(values[i]), [ f ], i + ',' + f);
        }
    }
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
coercion
TypeError
TypeError
TypeError
TypeError
string 1.2300000000e+2
TypeError
TypeError
TypeError
string 1.2300000000e+2
string 1.235e+4
string 1.235e+4
RangeError
RangeError
fractionDigits valueOf
string -Infinity
fractionDigits valueOf
string Infinity
fractionDigits valueOf
string NaN
fractionDigits valueOf
RangeError
===*/

/* Coercion order for a Number.prototype methods are a bit odd and different
 * from each other.  For toExponential():
 *
 * - 'this' coercion check (only accept plain number and Number objects)
 *   (V8 will coerce e.g. booleans to numbers)
 * - ToInteger(fractionDigits) but no range check yet
 * - special cases for NaN and +/- Infinity
 * - fractionDigits range check
 */

print('coercion');

function coercionTest() {
    var testnum = 12345.6789012345678901234567890;
    var fracObj;

    // this coercion

    test(undefined, [ 10 ]);
    test(null, [ 10 ]);
    test(true, [ 10 ]);
    test(false, [ 10 ]);
    test(123, [ 10 ]);
    test('foo', [ 10 ]);
    test([1,2], [ 10 ]);
    test({ foo: 1, bar: 2 }, [ 10 ]);
    test(new Number(123), [ 10 ]);

    // fractionDigits ToInteger() coercion

    test(new Number(testnum), [ '3.9' ]);  // -> 3
    test(new Number(testnum), [ 3 ]);

    test(new Number(testnum), [ -256*256*256*256 + 8 ]);  // invalid, no 32-bit wrap
    test(new Number(testnum), [ 256*256*256*256 + 8 ]);   // invalid, same

    // ToInteger(fractionDigits) coercion happens before NaN / infinity
    // check, but before fractionDigits range check

    fracObj = {
        toString: function() { print('fractionDigits toString'); return 1234; },  // invalid
        valueOf: function() { print('fractionDigits valueOf'); return 2345; }    // invalid
    };

    test(new Number(Number.NEGATIVE_INFINITY), [ fracObj ]);
    test(new Number(Number.POSITIVE_INFINITY), [ fracObj ]);
    test(new Number(Number.NaN), [ fracObj ]);

    test(new Number(12345), [ fracObj ]);  // for comparison -> RangeError
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
