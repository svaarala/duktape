/*
 *  toFixed()
 *
 *  NOTE: V8 will accept a boolean as the 'this' binding (only number and
 *  Number allowed in the specification).  Expect string is based on V8
 *  with manual fixes.
 */

function test(this_value, args, prefix_string) {
    var t;

    if (prefix_string === undefined) {
        prefix_string = '';
    } else {
        prefix_string += ' ';
    }

    try {
        t = Number.prototype.toFixed.apply(this_value, args);
        print(prefix_string + typeof t, t);
    } catch (e) {
        print(e.name);
    }
}

/*===
basic
string NaN
string 999999000000000032768
string 1e+21
string -999999000000000032768
string -1e+21
string -Infinity
string Infinity
RangeError
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
RangeError
RangeError
1,0 string -1e+100
1,1 string -1e+100
1,2 string -1e+100
1,3 string -1e+100
1,4 string -1e+100
1,5 string -1e+100
1,6 string -1e+100
1,7 string -1e+100
1,8 string -1e+100
1,9 string -1e+100
1,10 string -1e+100
1,11 string -1e+100
1,12 string -1e+100
1,13 string -1e+100
1,14 string -1e+100
1,15 string -1e+100
1,16 string -1e+100
1,17 string -1e+100
1,18 string -1e+100
1,19 string -1e+100
1,20 string -1e+100
RangeError
RangeError
2,0 string -1e+21
2,1 string -1e+21
2,2 string -1e+21
2,3 string -1e+21
2,4 string -1e+21
2,5 string -1e+21
2,6 string -1e+21
2,7 string -1e+21
2,8 string -1e+21
2,9 string -1e+21
2,10 string -1e+21
2,11 string -1e+21
2,12 string -1e+21
2,13 string -1e+21
2,14 string -1e+21
2,15 string -1e+21
2,16 string -1e+21
2,17 string -1e+21
2,18 string -1e+21
2,19 string -1e+21
2,20 string -1e+21
RangeError
RangeError
3,0 string -987654320999999995904
3,1 string -987654320999999995904.0
3,2 string -987654320999999995904.00
3,3 string -987654320999999995904.000
3,4 string -987654320999999995904.0000
3,5 string -987654320999999995904.00000
3,6 string -987654320999999995904.000000
3,7 string -987654320999999995904.0000000
3,8 string -987654320999999995904.00000000
3,9 string -987654320999999995904.000000000
3,10 string -987654320999999995904.0000000000
3,11 string -987654320999999995904.00000000000
3,12 string -987654320999999995904.000000000000
3,13 string -987654320999999995904.0000000000000
3,14 string -987654320999999995904.00000000000000
3,15 string -987654320999999995904.000000000000000
3,16 string -987654320999999995904.0000000000000000
3,17 string -987654320999999995904.00000000000000000
3,18 string -987654320999999995904.000000000000000000
3,19 string -987654320999999995904.0000000000000000000
3,20 string -987654320999999995904.00000000000000000000
RangeError
RangeError
4,0 string -12346
4,1 string -12345.7
4,2 string -12345.68
4,3 string -12345.679
4,4 string -12345.6789
4,5 string -12345.67890
4,6 string -12345.678901
4,7 string -12345.6789012
4,8 string -12345.67890123
4,9 string -12345.678901235
4,10 string -12345.6789012346
4,11 string -12345.67890123457
4,12 string -12345.678901234567
4,13 string -12345.6789012345671
4,14 string -12345.67890123456709
4,15 string -12345.678901234567093
4,16 string -12345.6789012345670926
4,17 string -12345.67890123456709262
4,18 string -12345.678901234567092615
4,19 string -12345.6789012345670926152
4,20 string -12345.67890123456709261518
RangeError
RangeError
5,0 string -0
5,1 string -0.1
5,2 string -0.12
5,3 string -0.123
5,4 string -0.1235
5,5 string -0.12346
5,6 string -0.123457
5,7 string -0.1234568
5,8 string -0.12345679
5,9 string -0.123456789
5,10 string -0.1234567890
5,11 string -0.12345678901
5,12 string -0.123456789012
5,13 string -0.1234567890123
5,14 string -0.12345678901235
5,15 string -0.123456789012346
5,16 string -0.1234567890123457
5,17 string -0.12345678901234568
5,18 string -0.123456789012345677
5,19 string -0.1234567890123456774
5,20 string -0.12345678901234567737
RangeError
RangeError
6,0 string -0
6,1 string -0.0
6,2 string -0.00
6,3 string -0.000
6,4 string -0.0000
6,5 string -0.00000
6,6 string -0.000000
6,7 string -0.0000000
6,8 string -0.00000000
6,9 string -0.000000000
6,10 string -0.0000000000
6,11 string -0.00000000000
6,12 string -0.000000000000
6,13 string -0.0000000000000
6,14 string -0.00000000000000
6,15 string -0.000000000000000
6,16 string -0.0000000000000000
6,17 string -0.00000000000000000
6,18 string -0.000000000000000000
6,19 string -0.0000000000000000000
6,20 string -0.00000000000000000000
RangeError
RangeError
7,0 string -0
7,1 string -0.0
7,2 string -0.00
7,3 string -0.000
7,4 string -0.0000
7,5 string -0.00000
7,6 string -0.000000
7,7 string -0.0000000
7,8 string -0.00000000
7,9 string -0.000000000
7,10 string -0.0000000000
7,11 string -0.00000000000
7,12 string -0.000000000000
7,13 string -0.0000000000000
7,14 string -0.00000000000000
7,15 string -0.000000000000000
7,16 string -0.0000000000000000
7,17 string -0.00000000000000000
7,18 string -0.000000000000000000
7,19 string -0.0000000000000000000
7,20 string -0.00000000000000000000
RangeError
RangeError
8,0 string -0
8,1 string -0.0
8,2 string -0.00
8,3 string -0.000
8,4 string -0.0000
8,5 string -0.00000
8,6 string -0.000000
8,7 string -0.0000000
8,8 string -0.00000000
8,9 string -0.000000000
8,10 string -0.0000000000
8,11 string -0.00000000000
8,12 string -0.000000000000
8,13 string -0.0000000000000
8,14 string -0.00000000000000
8,15 string -0.000000000000000
8,16 string -0.0000000000000000
8,17 string -0.00000000000000000
8,18 string -0.000000000000000000
8,19 string -0.0000000000000000000
8,20 string -0.00000000000000000000
RangeError
RangeError
9,0 string 0
9,1 string 0.0
9,2 string 0.00
9,3 string 0.000
9,4 string 0.0000
9,5 string 0.00000
9,6 string 0.000000
9,7 string 0.0000000
9,8 string 0.00000000
9,9 string 0.000000000
9,10 string 0.0000000000
9,11 string 0.00000000000
9,12 string 0.000000000000
9,13 string 0.0000000000000
9,14 string 0.00000000000000
9,15 string 0.000000000000000
9,16 string 0.0000000000000000
9,17 string 0.00000000000000000
9,18 string 0.000000000000000000
9,19 string 0.0000000000000000000
9,20 string 0.00000000000000000000
RangeError
RangeError
10,0 string 0
10,1 string 0.0
10,2 string 0.00
10,3 string 0.000
10,4 string 0.0000
10,5 string 0.00000
10,6 string 0.000000
10,7 string 0.0000000
10,8 string 0.00000000
10,9 string 0.000000000
10,10 string 0.0000000000
10,11 string 0.00000000000
10,12 string 0.000000000000
10,13 string 0.0000000000000
10,14 string 0.00000000000000
10,15 string 0.000000000000000
10,16 string 0.0000000000000000
10,17 string 0.00000000000000000
10,18 string 0.000000000000000000
10,19 string 0.0000000000000000000
10,20 string 0.00000000000000000000
RangeError
RangeError
11,0 string 0
11,1 string 0.0
11,2 string 0.00
11,3 string 0.000
11,4 string 0.0000
11,5 string 0.00000
11,6 string 0.000000
11,7 string 0.0000000
11,8 string 0.00000000
11,9 string 0.000000000
11,10 string 0.0000000000
11,11 string 0.00000000000
11,12 string 0.000000000000
11,13 string 0.0000000000000
11,14 string 0.00000000000000
11,15 string 0.000000000000000
11,16 string 0.0000000000000000
11,17 string 0.00000000000000000
11,18 string 0.000000000000000000
11,19 string 0.0000000000000000000
11,20 string 0.00000000000000000000
RangeError
RangeError
12,0 string 0
12,1 string 0.0
12,2 string 0.00
12,3 string 0.000
12,4 string 0.0000
12,5 string 0.00000
12,6 string 0.000000
12,7 string 0.0000000
12,8 string 0.00000000
12,9 string 0.000000000
12,10 string 0.0000000000
12,11 string 0.00000000000
12,12 string 0.000000000000
12,13 string 0.0000000000000
12,14 string 0.00000000000000
12,15 string 0.000000000000000
12,16 string 0.0000000000000000
12,17 string 0.00000000000000000
12,18 string 0.000000000000000000
12,19 string 0.0000000000000000000
12,20 string 0.00000000000000000000
RangeError
RangeError
13,0 string 0
13,1 string 0.0
13,2 string 0.00
13,3 string 0.000
13,4 string 0.0000
13,5 string 0.00000
13,6 string 0.000000
13,7 string 0.0000000
13,8 string 0.00000000
13,9 string 0.000000000
13,10 string 0.0000000000
13,11 string 0.00000000000
13,12 string 0.000000000000
13,13 string 0.0000000000000
13,14 string 0.00000000000000
13,15 string 0.000000000000000
13,16 string 0.0000000000000000
13,17 string 0.00000000000000000
13,18 string 0.000000000000000000
13,19 string 0.0000000000000000000
13,20 string 0.00000000000000000000
RangeError
RangeError
14,0 string 0
14,1 string 0.1
14,2 string 0.12
14,3 string 0.123
14,4 string 0.1235
14,5 string 0.12346
14,6 string 0.123457
14,7 string 0.1234568
14,8 string 0.12345679
14,9 string 0.123456789
14,10 string 0.1234567890
14,11 string 0.12345678901
14,12 string 0.123456789012
14,13 string 0.1234567890123
14,14 string 0.12345678901235
14,15 string 0.123456789012346
14,16 string 0.1234567890123457
14,17 string 0.12345678901234568
14,18 string 0.123456789012345677
14,19 string 0.1234567890123456774
14,20 string 0.12345678901234567737
RangeError
RangeError
15,0 string 12346
15,1 string 12345.7
15,2 string 12345.68
15,3 string 12345.679
15,4 string 12345.6789
15,5 string 12345.67890
15,6 string 12345.678901
15,7 string 12345.6789012
15,8 string 12345.67890123
15,9 string 12345.678901235
15,10 string 12345.6789012346
15,11 string 12345.67890123457
15,12 string 12345.678901234567
15,13 string 12345.6789012345671
15,14 string 12345.67890123456709
15,15 string 12345.678901234567093
15,16 string 12345.6789012345670926
15,17 string 12345.67890123456709262
15,18 string 12345.678901234567092615
15,19 string 12345.6789012345670926152
15,20 string 12345.67890123456709261518
RangeError
RangeError
16,0 string 987654320999999995904
16,1 string 987654320999999995904.0
16,2 string 987654320999999995904.00
16,3 string 987654320999999995904.000
16,4 string 987654320999999995904.0000
16,5 string 987654320999999995904.00000
16,6 string 987654320999999995904.000000
16,7 string 987654320999999995904.0000000
16,8 string 987654320999999995904.00000000
16,9 string 987654320999999995904.000000000
16,10 string 987654320999999995904.0000000000
16,11 string 987654320999999995904.00000000000
16,12 string 987654320999999995904.000000000000
16,13 string 987654320999999995904.0000000000000
16,14 string 987654320999999995904.00000000000000
16,15 string 987654320999999995904.000000000000000
16,16 string 987654320999999995904.0000000000000000
16,17 string 987654320999999995904.00000000000000000
16,18 string 987654320999999995904.000000000000000000
16,19 string 987654320999999995904.0000000000000000000
16,20 string 987654320999999995904.00000000000000000000
RangeError
RangeError
17,0 string 1e+21
17,1 string 1e+21
17,2 string 1e+21
17,3 string 1e+21
17,4 string 1e+21
17,5 string 1e+21
17,6 string 1e+21
17,7 string 1e+21
17,8 string 1e+21
17,9 string 1e+21
17,10 string 1e+21
17,11 string 1e+21
17,12 string 1e+21
17,13 string 1e+21
17,14 string 1e+21
17,15 string 1e+21
17,16 string 1e+21
17,17 string 1e+21
17,18 string 1e+21
17,19 string 1e+21
17,20 string 1e+21
RangeError
RangeError
18,0 string 1e+100
18,1 string 1e+100
18,2 string 1e+100
18,3 string 1e+100
18,4 string 1e+100
18,5 string 1e+100
18,6 string 1e+100
18,7 string 1e+100
18,8 string 1e+100
18,9 string 1e+100
18,10 string 1e+100
18,11 string 1e+100
18,12 string 1e+100
18,13 string 1e+100
18,14 string 1e+100
18,15 string 1e+100
18,16 string 1e+100
18,17 string 1e+100
18,18 string 1e+100
18,19 string 1e+100
18,20 string 1e+100
RangeError
RangeError
19,0 string Infinity
19,1 string Infinity
19,2 string Infinity
19,3 string Infinity
19,4 string Infinity
19,5 string Infinity
19,6 string Infinity
19,7 string Infinity
19,8 string Infinity
19,9 string Infinity
19,10 string Infinity
19,11 string Infinity
19,12 string Infinity
19,13 string Infinity
19,14 string Infinity
19,15 string Infinity
19,16 string Infinity
19,17 string Infinity
19,18 string Infinity
19,19 string Infinity
19,20 string Infinity
RangeError
RangeError
20,0 string NaN
20,1 string NaN
20,2 string NaN
20,3 string NaN
20,4 string NaN
20,5 string NaN
20,6 string NaN
20,7 string NaN
20,8 string NaN
20,9 string NaN
20,10 string NaN
20,11 string NaN
20,12 string NaN
20,13 string NaN
20,14 string NaN
20,15 string NaN
20,16 string NaN
20,17 string NaN
20,18 string NaN
20,19 string NaN
20,20 string NaN
RangeError
===*/

print('basic');

function basicTest() {
    var i, f;

    var values = [
        Number.NEGATIVE_INFINITY,
        -1e100,
        -1e21,
        -0.987654321e21,
        -12345.6789012345678901234567890,
        -0.123456789012345678901234567890,
        -0.123456789012345678901234567890e-20,
        -0.123456789012345678901234567890e-21,
        -0.123456789012345678901234567890e-22,
        -0,
        +0,
        0.123456789012345678901234567890e-22,
        0.123456789012345678901234567890e-21,
        0.123456789012345678901234567890e-20,
        0.123456789012345678901234567890,
        12345.6789012345678901234567890,
        0.987654321e21,
        1e21,
        1e100,
        Number.POSITIVE_INFINITY,
        Number.NaN
    ];

    // Step 4: special handling for NaN

    test(new Number(Number.NaN), []);

    // Step 7: if abs(number) > 10^21, fall back to ToString()
    // (this also covers infinite values)

    test(new Number(0.999999e21), []);   // below
    test(new Number(1e21), []);          // above

    test(new Number(-0.999999e21), []);  // below
    test(new Number(-1e21), []);         // above

    test(new Number(Number.NEGATIVE_INFINITY), []);
    test(new Number(Number.POSITIVE_INFINITY), []);

    // test a bunch of value and fractionDigits combinations

    for (i = 0; i < values.length; i++) {
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
string 123
TypeError
TypeError
TypeError
string 123
frac -3
RangeError
frac -2
RangeError
frac -1
RangeError
frac 0
string 12346
frac 1
string 12345.7
frac 2
string 12345.68
frac 3
string 12345.679
frac 4
string 12345.6789
frac 5
string 12345.67890
frac 6
string 12345.678901
frac 7
string 12345.6789012
frac 8
string 12345.67890123
frac 9
string 12345.678901235
frac 10
string 12345.6789012346
frac 11
string 12345.67890123457
frac 12
string 12345.678901234567
frac 13
string 12345.6789012345671
frac 14
string 12345.67890123456709
frac 15
string 12345.678901234567093
frac 16
string 12345.6789012345670926
frac 17
string 12345.67890123456709262
frac 18
string 12345.678901234567092615
frac 19
string 12345.6789012345670926152
frac 20
string 12345.67890123456709261518
frac 21
RangeError
frac 22
RangeError
frac 23
RangeError
string 12345.679
RangeError
RangeError
fractionDigits valueOf
string 12345.678901
frac -1
RangeError
RangeError
RangeError
RangeError
RangeError
RangeError
RangeError
RangeError
RangeError
frac 0
TypeError
TypeError
TypeError
TypeError
string 123
TypeError
TypeError
TypeError
string 123
frac 20
TypeError
TypeError
TypeError
TypeError
string 123.00000000000000000000
TypeError
TypeError
TypeError
string 123.00000000000000000000
frac 21
RangeError
RangeError
RangeError
RangeError
RangeError
RangeError
RangeError
RangeError
RangeError
===*/

/* Like some other Number.prototype methods, toFixed() has a weird coercion
 * order:
 *
 *  - ToInteger(fractionDigits) followed by a range check for the result
 *  - Only after that, a check for 'this' binding
 *    (V8 coerces e.g. booleans to numbers, only number/Number should be accepted)
 */

print('coercion');

function coercionTest() {
    var i, f;
    var testnum = 12345.6789012345678901234567890;

    // this coercion - occurs *after* fractionDigits coercion (undefined = 0 here)
    // (only plain number and a Number instance allowed as a valid 'this' binding)

    test(undefined, []);
    test(null, []);
    test(true, []);
    test(false, []);
    test(123, []);
    test('foo', []);
    test([1,2], []);
    test({ foo: 1, bar: 2 }, []);
    test(new Number(123), []);

    // fractionDigits, ToInteger() coercion, [0,20] range

    for (i = -3; i <= 23; i++) {
        print('frac', i);
        test(new Number(testnum), [ i ]);
    }

    test(new Number(testnum), [ '3.9' ]);  // -> 3
    test(new Number(testnum), [ -256*256*256*256 + 3.9 ]);  // -> invalid, no 32-bit wrap
    test(new Number(testnum), [ 256*256*256*256 + 3.9 ]);   // -> invalid, same

    test(new Number(testnum), [ {
        toString: function() { print('fractionDigits toString'); return 10; },
        valueOf: function() { print('fractionDigits valueOf'); return 6; },
    } ]);

    // an invalid fractionDigits value is detected (and reported as RangeError)
    // before an invalid 'this' binding (this differs from some other Number
    // built-in methods)

    var frac_values = [ -1, 0, 20, 21 ];
    for (i = 0; i < frac_values.length; i++) {
        f = frac_values[i];
        print('frac', f);
        test(undefined, [ f ]);
        test(null, [ f ]);
        test(true, [ f ]);
        test(false, [ f ]);
        test(123, [ f ]);
        test('foo', [ f ]);
        test([1,2], [ f ]);
        test({ foo: 1, bar: 2 }, [ f ]);
        test(new Number(123), [ f ]);
    }
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
