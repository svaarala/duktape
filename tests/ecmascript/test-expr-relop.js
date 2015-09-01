/*
 *  Relational operators (E5 Section 11.8).
 */

/*===
0 0 false false true true
0 1 true false true false
0 2 true false true false
0 3 true false true false
0 4 true false true false
0 5 true false true false
0 6 true false true false
0 7 true false true false
0 8 false false false false
0 9 true false true false
0 10 false false false false
0 11 false false false false
0 12 false false false false
0 13 false false false false
1 0 false true false true
1 1 false false true true
1 2 true false true false
1 3 true false true false
1 4 true false true false
1 5 true false true false
1 6 true false true false
1 7 true false true false
1 8 false false false false
1 9 true false true false
1 10 false false false false
1 11 false false false false
1 12 false false false false
1 13 false false false false
2 0 false true false true
2 1 false true false true
2 2 false false true true
2 3 true false true false
2 4 true false true false
2 5 true false true false
2 6 true false true false
2 7 true false true false
2 8 false false false false
2 9 true false true false
2 10 false false false false
2 11 false false false false
2 12 false false false false
2 13 false false false false
3 0 false true false true
3 1 false true false true
3 2 false true false true
3 3 false false true true
3 4 false false true true
3 5 true false true false
3 6 true false true false
3 7 true false true false
3 8 false false false false
3 9 false false true true
3 10 false false false false
3 11 false false false false
3 12 false false false false
3 13 false false false false
4 0 false true false true
4 1 false true false true
4 2 false true false true
4 3 false false true true
4 4 false false true true
4 5 true false true false
4 6 true false true false
4 7 true false true false
4 8 false false false false
4 9 false false true true
4 10 false false false false
4 11 false false false false
4 12 false false false false
4 13 false false false false
5 0 false true false true
5 1 false true false true
5 2 false true false true
5 3 false true false true
5 4 false true false true
5 5 false false true true
5 6 true false true false
5 7 true false true false
5 8 false false false false
5 9 false true false true
5 10 false false false false
5 11 false false false false
5 12 false false false false
5 13 false false false false
6 0 false true false true
6 1 false true false true
6 2 false true false true
6 3 false true false true
6 4 false true false true
6 5 false true false true
6 6 false false true true
6 7 true false true false
6 8 false false false false
6 9 false true false true
6 10 false false false false
6 11 false false false false
6 12 false false false false
6 13 false false false false
7 0 false true false true
7 1 false true false true
7 2 false true false true
7 3 false true false true
7 4 false true false true
7 5 false true false true
7 6 false true false true
7 7 false false true true
7 8 false false false false
7 9 false true false true
7 10 false false false false
7 11 false false false false
7 12 false false false false
7 13 false false false false
8 0 false false false false
8 1 false false false false
8 2 false false false false
8 3 false false false false
8 4 false false false false
8 5 false false false false
8 6 false false false false
8 7 false false false false
8 8 false false false false
8 9 false false false false
8 10 false false false false
8 11 false false false false
8 12 false false false false
8 13 false false false false
9 0 false true false true
9 1 false true false true
9 2 false true false true
9 3 false false true true
9 4 false false true true
9 5 true false true false
9 6 true false true false
9 7 true false true false
9 8 false false false false
9 9 false false true true
9 10 true false true false
9 11 true false true false
9 12 true false true false
9 13 true false true false
10 0 false false false false
10 1 false false false false
10 2 false false false false
10 3 false false false false
10 4 false false false false
10 5 false false false false
10 6 false false false false
10 7 false false false false
10 8 false false false false
10 9 false true false true
10 10 false false true true
10 11 true false true false
10 12 true false true false
10 13 true false true false
11 0 false false false false
11 1 false false false false
11 2 false false false false
11 3 false false false false
11 4 false false false false
11 5 false false false false
11 6 false false false false
11 7 false false false false
11 8 false false false false
11 9 false true false true
11 10 false true false true
11 11 false false true true
11 12 true false true false
11 13 true false true false
12 0 false false false false
12 1 false false false false
12 2 false false false false
12 3 false false false false
12 4 false false false false
12 5 false false false false
12 6 false false false false
12 7 false false false false
12 8 false false false false
12 9 false true false true
12 10 false true false true
12 11 false true false true
12 12 false false true true
12 13 true false true false
13 0 false false false false
13 1 false false false false
13 2 false false false false
13 3 false false false false
13 4 false false false false
13 5 false false false false
13 6 false false false false
13 7 false false false false
13 8 false false false false
13 9 false true false true
13 10 false true false true
13 11 false true false true
13 12 false true false true
13 13 false false true true
0 0 TypeError TypeError
0 1 TypeError TypeError
0 2 TypeError TypeError
0 3 TypeError false
0 4 false false
0 5 false false
0 6 false false
0 7 TypeError false
1 0 TypeError TypeError
1 1 TypeError TypeError
1 2 TypeError TypeError
1 3 TypeError false
1 4 false false
1 5 false false
1 6 false false
1 7 TypeError false
2 0 TypeError TypeError
2 1 TypeError TypeError
2 2 TypeError TypeError
2 3 TypeError false
2 4 false false
2 5 false false
2 6 false false
2 7 TypeError true
3 0 TypeError TypeError
3 1 TypeError TypeError
3 2 TypeError TypeError
3 3 TypeError false
3 4 true false
3 5 false false
3 6 false false
3 7 TypeError false
4 0 TypeError TypeError
4 1 TypeError TypeError
4 2 TypeError TypeError
4 3 TypeError false
4 4 false false
4 5 false false
4 6 false false
4 7 TypeError false
5 0 TypeError TypeError
5 1 TypeError TypeError
5 2 TypeError TypeError
5 3 TypeError false
5 4 false false
5 5 false false
5 6 false false
5 7 TypeError false
6 0 TypeError TypeError
6 1 TypeError TypeError
6 2 TypeError TypeError
6 3 TypeError false
6 4 false false
6 5 false false
6 6 false false
6 7 TypeError false
7 0 TypeError TypeError
7 1 TypeError TypeError
7 2 TypeError TypeError
7 3 TypeError false
7 4 false false
7 5 false false
7 6 false false
7 7 TypeError false
===*/

/* Number and string comparisons using greater/less than (or equal). */

var testValues = [
    -1/0, -1e9, -1, -0, +0, +1, 1e9, 1/0, 0/0,

    '', 'foo', 'foo\0', 'foo\u1234', 'foo\u4321',
];

testValues.forEach(function (v1, i1) {
    testValues.forEach(function (v2, i2) {
        print(i1, i2, v1 < v2, v1 > v2, v1 <= v2, v1 >= v2);
    });
});

/* Instanceof and in.  Some test value combinations are not all that
 * meaningful, but brute forcing the combinations is still OK.
 */

var testValues = [
    1e9, '', 'foo',
    new Error('my error'),
    Error, Number, String,
    { foo: 123 }
];

testValues.forEach(function (v1, i1) {
    testValues.forEach(function (v2, i2) {
        var res1, res2;
        try { res1 = v1 instanceof v2; } catch (e) { res1 = String(e.name); }
        try { res2 = v1 in v2; } catch (e) { res2 = String(e.name); }
        print(i1, i2, res1, res2);
    });
});
