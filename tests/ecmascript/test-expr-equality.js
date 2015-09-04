/*
 *  Equality operators (E5 Section 11.9).
 */

/*===
left
right
false
left
right
true
left
right
false
left
right
true
===*/

/* Evaluation order is left-to-right */
print( (print('left'), 1) == (print('right'), 2) );
print( (print('left'), 1) != (print('right'), 2) );
print( (print('left'), 1) === (print('right'), 2) );
print( (print('left'), 1) !== (print('right'), 2) );

/*===
0 0 true false true false
0 1 true false false true
0 2 false true false true
0 3 false true false true
0 4 false true false true
0 5 false true false true
0 6 false true false true
0 7 false true false true
0 8 false true false true
0 9 false true false true
0 10 false true false true
0 11 false true false true
0 12 false true false true
0 13 false true false true
0 14 false true false true
0 15 false true false true
0 16 false true false true
0 17 false true false true
1 0 true false false true
1 1 true false true false
1 2 false true false true
1 3 false true false true
1 4 false true false true
1 5 false true false true
1 6 false true false true
1 7 false true false true
1 8 false true false true
1 9 false true false true
1 10 false true false true
1 11 false true false true
1 12 false true false true
1 13 false true false true
1 14 false true false true
1 15 false true false true
1 16 false true false true
1 17 false true false true
2 0 false true false true
2 1 false true false true
2 2 true false true false
2 3 false true false true
2 4 false true false true
2 5 false true false true
2 6 false true false true
2 7 false true false true
2 8 false true false true
2 9 true false false true
2 10 false true false true
2 11 false true false true
2 12 false true false true
2 13 false true false true
2 14 false true false true
2 15 false true false true
2 16 false true false true
2 17 false true false true
3 0 false true false true
3 1 false true false true
3 2 false true false true
3 3 true false true false
3 4 false true false true
3 5 false true false true
3 6 false true false true
3 7 true false false true
3 8 true false false true
3 9 false true false true
3 10 false true false true
3 11 false true false true
3 12 false true false true
3 13 true false false true
3 14 false true false true
3 15 true false false true
3 16 false true false true
3 17 false true false true
4 0 false true false true
4 1 false true false true
4 2 false true false true
4 3 false true false true
4 4 true false true false
4 5 false true false true
4 6 false true false true
4 7 false true false true
4 8 false true false true
4 9 false true false true
4 10 false true false true
4 11 false true false true
4 12 false true false true
4 13 false true false true
4 14 false true false true
4 15 false true false true
4 16 false true false true
4 17 false true false true
5 0 false true false true
5 1 false true false true
5 2 false true false true
5 3 false true false true
5 4 false true false true
5 5 true false true false
5 6 false true false true
5 7 false true false true
5 8 false true false true
5 9 false true false true
5 10 false true false true
5 11 false true false true
5 12 false true false true
5 13 false true false true
5 14 false true false true
5 15 false true false true
5 16 false true false true
5 17 false true false true
6 0 false true false true
6 1 false true false true
6 2 false true false true
6 3 false true false true
6 4 false true false true
6 5 false true false true
6 6 true false true false
6 7 false true false true
6 8 false true false true
6 9 false true false true
6 10 false true false true
6 11 false true false true
6 12 false true false true
6 13 false true false true
6 14 false true false true
6 15 false true false true
6 16 false true false true
6 17 false true false true
7 0 false true false true
7 1 false true false true
7 2 false true false true
7 3 true false false true
7 4 false true false true
7 5 false true false true
7 6 false true false true
7 7 true false true false
7 8 true false true false
7 9 false true false true
7 10 false true false true
7 11 false true false true
7 12 false true false true
7 13 true false false true
7 14 false true false true
7 15 true false false true
7 16 false true false true
7 17 false true false true
8 0 false true false true
8 1 false true false true
8 2 false true false true
8 3 true false false true
8 4 false true false true
8 5 false true false true
8 6 false true false true
8 7 true false true false
8 8 true false true false
8 9 false true false true
8 10 false true false true
8 11 false true false true
8 12 false true false true
8 13 true false false true
8 14 false true false true
8 15 true false false true
8 16 false true false true
8 17 false true false true
9 0 false true false true
9 1 false true false true
9 2 true false false true
9 3 false true false true
9 4 false true false true
9 5 false true false true
9 6 false true false true
9 7 false true false true
9 8 false true false true
9 9 true false true false
9 10 false true false true
9 11 false true false true
9 12 false true false true
9 13 false true false true
9 14 false true false true
9 15 false true false true
9 16 false true false true
9 17 false true false true
10 0 false true false true
10 1 false true false true
10 2 false true false true
10 3 false true false true
10 4 false true false true
10 5 false true false true
10 6 false true false true
10 7 false true false true
10 8 false true false true
10 9 false true false true
10 10 true false true false
10 11 false true false true
10 12 false true false true
10 13 false true false true
10 14 false true false true
10 15 false true false true
10 16 false true false true
10 17 false true false true
11 0 false true false true
11 1 false true false true
11 2 false true false true
11 3 false true false true
11 4 false true false true
11 5 false true false true
11 6 false true false true
11 7 false true false true
11 8 false true false true
11 9 false true false true
11 10 false true false true
11 11 true false true false
11 12 false true false true
11 13 false true false true
11 14 false true false true
11 15 false true false true
11 16 false true false true
11 17 false true false true
12 0 false true false true
12 1 false true false true
12 2 false true false true
12 3 false true false true
12 4 false true false true
12 5 false true false true
12 6 false true false true
12 7 false true false true
12 8 false true false true
12 9 false true false true
12 10 false true false true
12 11 false true false true
12 12 false true false true
12 13 false true false true
12 14 false true false true
12 15 false true false true
12 16 false true false true
12 17 false true false true
13 0 false true false true
13 1 false true false true
13 2 false true false true
13 3 true false false true
13 4 false true false true
13 5 false true false true
13 6 false true false true
13 7 true false false true
13 8 true false false true
13 9 false true false true
13 10 false true false true
13 11 false true false true
13 12 false true false true
13 13 true false true false
13 14 false true false true
13 15 true false false true
13 16 false true false true
13 17 false true false true
14 0 false true false true
14 1 false true false true
14 2 false true false true
14 3 false true false true
14 4 false true false true
14 5 false true false true
14 6 false true false true
14 7 false true false true
14 8 false true false true
14 9 false true false true
14 10 false true false true
14 11 false true false true
14 12 false true false true
14 13 false true false true
14 14 true false true false
14 15 false true false true
14 16 false true false true
14 17 false true false true
15 0 false true false true
15 1 false true false true
15 2 false true false true
15 3 true false false true
15 4 false true false true
15 5 false true false true
15 6 false true false true
15 7 true false false true
15 8 true false false true
15 9 false true false true
15 10 false true false true
15 11 false true false true
15 12 false true false true
15 13 true false false true
15 14 false true false true
15 15 true false true false
15 16 false true false true
15 17 false true false true
16 0 false true false true
16 1 false true false true
16 2 false true false true
16 3 false true false true
16 4 false true false true
16 5 false true false true
16 6 false true false true
16 7 false true false true
16 8 false true false true
16 9 false true false true
16 10 false true false true
16 11 false true false true
16 12 false true false true
16 13 false true false true
16 14 false true false true
16 15 false true false true
16 16 true false true false
16 17 false true false true
17 0 false true false true
17 1 false true false true
17 2 false true false true
17 3 false true false true
17 4 false true false true
17 5 false true false true
17 6 false true false true
17 7 false true false true
17 8 false true false true
17 9 false true false true
17 10 false true false true
17 11 false true false true
17 12 false true false true
17 13 false true false true
17 14 false true false true
17 15 false true false true
17 16 false true false true
17 17 true false true false
===*/

var testValues = [
    undefined, null, true, false,

    -1/0, -1e9, -1, -0, +0, +1, 1e9, 1/0, 0/0,

    '', 'foo',

    [], {},
    function test() {}
];

testValues.forEach(function (v1, i1) {
    testValues.forEach(function (v2, i2) {
        print(i1, i2, v1 == v2, v1 != v2, v1 === v2, v1 !== v2);
    });
});
