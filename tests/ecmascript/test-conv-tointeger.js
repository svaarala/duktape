/*
 *  ToInteger() (E5 Section 9.4).
 *
 *  ToInteger() appears in many places but its result is difficult to get out
 *  unaltered.  Date.UTC() allows that for TimeClip() range if every argument
 *  is given as 0 except milliseconds.  The arguments will then be ToNumber()
 *  coerced (which is a no-op), and milliseconds will then go through:
 *
 *    - MakeTime: will return 0 + 0 + 0 + ToInteger(ms)
 *    - MakeDate: year1900_in_msec + ToInteger(ms)
 *    - TimeClip: year1900_in_msec + ToInteger(ms) or
 *                year1900_in_msec + (+0) + ToInteger(ms),
 *                or NaN if abs(ToInteger(ms)) > 8.64e15
 *
 *  Negative zero will not survive this process but will be converted to a
 *  positive zero.  The year 1900 replacement happens because of the explicit
 *  year handling step in E5 Section 15.9.4.3, step 8.
 */

/*===
0 -1000000000000
1 -10000000000
2 -1000000000
3 -100
4 -100
5 -100
6 -1
7 0
8 0
9 1
10 100
11 100
12 100
13 1000000000
14 10000000000
15 1000000000000
===*/

function simulateToNumber(v) {
    var year1900 = Date.UTC(0, 0, 0, 0, 0, 0, 0).valueOf();
    return Date.UTC(0, 0, 0, 0, 0, 0, v).valueOf() - year1900;
}

function test() {
    [ -1e12, -1e10, -1e9, -100.9, -100.5, -100.1, -1, -0, +0, 1, 100.1, 100.5, 100.9,
      1e9, 1e10, 1e12 ].forEach(function (v, i) {
        print(i, simulateToNumber(v));
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
