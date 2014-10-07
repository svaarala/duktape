/*
 *  String.prototype.slice() tests.
 *
 *  The basic difference between slice() and substring() is that slice
 *  won't swap the arguments if start is higher than end.
 */

/*===
basic
noargs string 6 foobar
-Infinity -Infinity string 0 
-Infinity -7 string 0 
-Infinity -6 string 0 
-Infinity -5 string 1 f
-Infinity -4 string 2 fo
-Infinity -3 string 3 foo
-Infinity -3 string 3 foo
-Infinity 0 string 0 
-Infinity 0 string 0 
-Infinity 1 string 1 f
-Infinity 2 string 2 fo
-Infinity 3 string 3 foo
-Infinity 4 string 4 foob
-Infinity 5 string 5 fooba
-Infinity 6 string 6 foobar
-Infinity 7 string 6 foobar
-Infinity Infinity string 6 foobar
-Infinity NaN string 0 
-7 -Infinity string 0 
-7 -7 string 0 
-7 -6 string 0 
-7 -5 string 1 f
-7 -4 string 2 fo
-7 -3 string 3 foo
-7 -3 string 3 foo
-7 0 string 0 
-7 0 string 0 
-7 1 string 1 f
-7 2 string 2 fo
-7 3 string 3 foo
-7 4 string 4 foob
-7 5 string 5 fooba
-7 6 string 6 foobar
-7 7 string 6 foobar
-7 Infinity string 6 foobar
-7 NaN string 0 
-6 -Infinity string 0 
-6 -7 string 0 
-6 -6 string 0 
-6 -5 string 1 f
-6 -4 string 2 fo
-6 -3 string 3 foo
-6 -3 string 3 foo
-6 0 string 0 
-6 0 string 0 
-6 1 string 1 f
-6 2 string 2 fo
-6 3 string 3 foo
-6 4 string 4 foob
-6 5 string 5 fooba
-6 6 string 6 foobar
-6 7 string 6 foobar
-6 Infinity string 6 foobar
-6 NaN string 0 
-5 -Infinity string 0 
-5 -7 string 0 
-5 -6 string 0 
-5 -5 string 0 
-5 -4 string 1 o
-5 -3 string 2 oo
-5 -3 string 2 oo
-5 0 string 0 
-5 0 string 0 
-5 1 string 0 
-5 2 string 1 o
-5 3 string 2 oo
-5 4 string 3 oob
-5 5 string 4 ooba
-5 6 string 5 oobar
-5 7 string 5 oobar
-5 Infinity string 5 oobar
-5 NaN string 0 
-4 -Infinity string 0 
-4 -7 string 0 
-4 -6 string 0 
-4 -5 string 0 
-4 -4 string 0 
-4 -3 string 1 o
-4 -3 string 1 o
-4 0 string 0 
-4 0 string 0 
-4 1 string 0 
-4 2 string 0 
-4 3 string 1 o
-4 4 string 2 ob
-4 5 string 3 oba
-4 6 string 4 obar
-4 7 string 4 obar
-4 Infinity string 4 obar
-4 NaN string 0 
-3 -Infinity string 0 
-3 -7 string 0 
-3 -6 string 0 
-3 -5 string 0 
-3 -4 string 0 
-3 -3 string 0 
-3 -3 string 0 
-3 0 string 0 
-3 0 string 0 
-3 1 string 0 
-3 2 string 0 
-3 3 string 0 
-3 4 string 1 b
-3 5 string 2 ba
-3 6 string 3 bar
-3 7 string 3 bar
-3 Infinity string 3 bar
-3 NaN string 0 
-3 -Infinity string 0 
-3 -7 string 0 
-3 -6 string 0 
-3 -5 string 0 
-3 -4 string 0 
-3 -3 string 0 
-3 -3 string 0 
-3 0 string 0 
-3 0 string 0 
-3 1 string 0 
-3 2 string 0 
-3 3 string 0 
-3 4 string 1 b
-3 5 string 2 ba
-3 6 string 3 bar
-3 7 string 3 bar
-3 Infinity string 3 bar
-3 NaN string 0 
0 -Infinity string 0 
0 -7 string 0 
0 -6 string 0 
0 -5 string 1 f
0 -4 string 2 fo
0 -3 string 3 foo
0 -3 string 3 foo
0 0 string 0 
0 0 string 0 
0 1 string 1 f
0 2 string 2 fo
0 3 string 3 foo
0 4 string 4 foob
0 5 string 5 fooba
0 6 string 6 foobar
0 7 string 6 foobar
0 Infinity string 6 foobar
0 NaN string 0 
0 -Infinity string 0 
0 -7 string 0 
0 -6 string 0 
0 -5 string 1 f
0 -4 string 2 fo
0 -3 string 3 foo
0 -3 string 3 foo
0 0 string 0 
0 0 string 0 
0 1 string 1 f
0 2 string 2 fo
0 3 string 3 foo
0 4 string 4 foob
0 5 string 5 fooba
0 6 string 6 foobar
0 7 string 6 foobar
0 Infinity string 6 foobar
0 NaN string 0 
1 -Infinity string 0 
1 -7 string 0 
1 -6 string 0 
1 -5 string 0 
1 -4 string 1 o
1 -3 string 2 oo
1 -3 string 2 oo
1 0 string 0 
1 0 string 0 
1 1 string 0 
1 2 string 1 o
1 3 string 2 oo
1 4 string 3 oob
1 5 string 4 ooba
1 6 string 5 oobar
1 7 string 5 oobar
1 Infinity string 5 oobar
1 NaN string 0 
2 -Infinity string 0 
2 -7 string 0 
2 -6 string 0 
2 -5 string 0 
2 -4 string 0 
2 -3 string 1 o
2 -3 string 1 o
2 0 string 0 
2 0 string 0 
2 1 string 0 
2 2 string 0 
2 3 string 1 o
2 4 string 2 ob
2 5 string 3 oba
2 6 string 4 obar
2 7 string 4 obar
2 Infinity string 4 obar
2 NaN string 0 
3 -Infinity string 0 
3 -7 string 0 
3 -6 string 0 
3 -5 string 0 
3 -4 string 0 
3 -3 string 0 
3 -3 string 0 
3 0 string 0 
3 0 string 0 
3 1 string 0 
3 2 string 0 
3 3 string 0 
3 4 string 1 b
3 5 string 2 ba
3 6 string 3 bar
3 7 string 3 bar
3 Infinity string 3 bar
3 NaN string 0 
4 -Infinity string 0 
4 -7 string 0 
4 -6 string 0 
4 -5 string 0 
4 -4 string 0 
4 -3 string 0 
4 -3 string 0 
4 0 string 0 
4 0 string 0 
4 1 string 0 
4 2 string 0 
4 3 string 0 
4 4 string 0 
4 5 string 1 a
4 6 string 2 ar
4 7 string 2 ar
4 Infinity string 2 ar
4 NaN string 0 
5 -Infinity string 0 
5 -7 string 0 
5 -6 string 0 
5 -5 string 0 
5 -4 string 0 
5 -3 string 0 
5 -3 string 0 
5 0 string 0 
5 0 string 0 
5 1 string 0 
5 2 string 0 
5 3 string 0 
5 4 string 0 
5 5 string 0 
5 6 string 1 r
5 7 string 1 r
5 Infinity string 1 r
5 NaN string 0 
6 -Infinity string 0 
6 -7 string 0 
6 -6 string 0 
6 -5 string 0 
6 -4 string 0 
6 -3 string 0 
6 -3 string 0 
6 0 string 0 
6 0 string 0 
6 1 string 0 
6 2 string 0 
6 3 string 0 
6 4 string 0 
6 5 string 0 
6 6 string 0 
6 7 string 0 
6 Infinity string 0 
6 NaN string 0 
7 -Infinity string 0 
7 -7 string 0 
7 -6 string 0 
7 -5 string 0 
7 -4 string 0 
7 -3 string 0 
7 -3 string 0 
7 0 string 0 
7 0 string 0 
7 1 string 0 
7 2 string 0 
7 3 string 0 
7 4 string 0 
7 5 string 0 
7 6 string 0 
7 7 string 0 
7 Infinity string 0 
7 NaN string 0 
Infinity -Infinity string 0 
Infinity -7 string 0 
Infinity -6 string 0 
Infinity -5 string 0 
Infinity -4 string 0 
Infinity -3 string 0 
Infinity -3 string 0 
Infinity 0 string 0 
Infinity 0 string 0 
Infinity 1 string 0 
Infinity 2 string 0 
Infinity 3 string 0 
Infinity 4 string 0 
Infinity 5 string 0 
Infinity 6 string 0 
Infinity 7 string 0 
Infinity Infinity string 0 
Infinity NaN string 0 
NaN -Infinity string 0 
NaN -7 string 0 
NaN -6 string 0 
NaN -5 string 1 f
NaN -4 string 2 fo
NaN -3 string 3 foo
NaN -3 string 3 foo
NaN 0 string 0 
NaN 0 string 0 
NaN 1 string 1 f
NaN 2 string 2 fo
NaN 3 string 3 foo
NaN 4 string 4 foob
NaN 5 string 5 fooba
NaN 6 string 6 foobar
NaN 7 string 6 foobar
NaN Infinity string 6 foobar
NaN NaN string 0 
===*/

print('basic');

function basicTest() {
    var str = 'foobar';
    var numbers = [
        Number.NEGATIVE_INFINITY, -7, -6, -5, -4, -3, -2 -1,
        -0, +0, 1, 2, 3, 4, 5, 6, 7, Number.POSITIVE_INFINITY, Number.NaN
    ];
    var t;

    function test(s, e) {
        var x = str.slice(s, e);
        print(s, e, typeof x, x.length, x);
    }

    t = str.slice();
    print('noargs', typeof t, t.length, t);

    // all combinations of 'numbers'
    for (i = 0; i < numbers.length; i++) {
        for (j = 0; j < numbers.length; j++) {
            test(numbers[i], numbers[j]);
        }
    }
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
non-bmp
-Infinity -Infinity string 0 
-Infinity -10 string 0 
-Infinity -9 string 1 102
-Infinity -8 string 2 102 111
-Infinity -7 string 3 102 111 111
-Infinity -6 string 4 102 111 111 128
-Infinity -5 string 5 102 111 111 128 2047
-Infinity -7 string 3 102 111 111
-Infinity -2 string 8 102 111 111 128 2047 4660 65535 98
-Infinity -1 string 9 102 111 111 128 2047 4660 65535 98 97
-Infinity 0 string 0 
-Infinity 0 string 0 
-Infinity 1 string 1 102
-Infinity 2 string 2 102 111
-Infinity 3 string 3 102 111 111
-Infinity 4 string 4 102 111 111 128
-Infinity 5 string 5 102 111 111 128 2047
-Infinity 6 string 6 102 111 111 128 2047 4660
-Infinity 7 string 7 102 111 111 128 2047 4660 65535
-Infinity 8 string 8 102 111 111 128 2047 4660 65535 98
-Infinity 9 string 9 102 111 111 128 2047 4660 65535 98 97
-Infinity 10 string 10 102 111 111 128 2047 4660 65535 98 97 114
-Infinity Infinity string 10 102 111 111 128 2047 4660 65535 98 97 114
-Infinity NaN string 0 
-10 -Infinity string 0 
-10 -10 string 0 
-10 -9 string 1 102
-10 -8 string 2 102 111
-10 -7 string 3 102 111 111
-10 -6 string 4 102 111 111 128
-10 -5 string 5 102 111 111 128 2047
-10 -7 string 3 102 111 111
-10 -2 string 8 102 111 111 128 2047 4660 65535 98
-10 -1 string 9 102 111 111 128 2047 4660 65535 98 97
-10 0 string 0 
-10 0 string 0 
-10 1 string 1 102
-10 2 string 2 102 111
-10 3 string 3 102 111 111
-10 4 string 4 102 111 111 128
-10 5 string 5 102 111 111 128 2047
-10 6 string 6 102 111 111 128 2047 4660
-10 7 string 7 102 111 111 128 2047 4660 65535
-10 8 string 8 102 111 111 128 2047 4660 65535 98
-10 9 string 9 102 111 111 128 2047 4660 65535 98 97
-10 10 string 10 102 111 111 128 2047 4660 65535 98 97 114
-10 Infinity string 10 102 111 111 128 2047 4660 65535 98 97 114
-10 NaN string 0 
-9 -Infinity string 0 
-9 -10 string 0 
-9 -9 string 0 
-9 -8 string 1 111
-9 -7 string 2 111 111
-9 -6 string 3 111 111 128
-9 -5 string 4 111 111 128 2047
-9 -7 string 2 111 111
-9 -2 string 7 111 111 128 2047 4660 65535 98
-9 -1 string 8 111 111 128 2047 4660 65535 98 97
-9 0 string 0 
-9 0 string 0 
-9 1 string 0 
-9 2 string 1 111
-9 3 string 2 111 111
-9 4 string 3 111 111 128
-9 5 string 4 111 111 128 2047
-9 6 string 5 111 111 128 2047 4660
-9 7 string 6 111 111 128 2047 4660 65535
-9 8 string 7 111 111 128 2047 4660 65535 98
-9 9 string 8 111 111 128 2047 4660 65535 98 97
-9 10 string 9 111 111 128 2047 4660 65535 98 97 114
-9 Infinity string 9 111 111 128 2047 4660 65535 98 97 114
-9 NaN string 0 
-8 -Infinity string 0 
-8 -10 string 0 
-8 -9 string 0 
-8 -8 string 0 
-8 -7 string 1 111
-8 -6 string 2 111 128
-8 -5 string 3 111 128 2047
-8 -7 string 1 111
-8 -2 string 6 111 128 2047 4660 65535 98
-8 -1 string 7 111 128 2047 4660 65535 98 97
-8 0 string 0 
-8 0 string 0 
-8 1 string 0 
-8 2 string 0 
-8 3 string 1 111
-8 4 string 2 111 128
-8 5 string 3 111 128 2047
-8 6 string 4 111 128 2047 4660
-8 7 string 5 111 128 2047 4660 65535
-8 8 string 6 111 128 2047 4660 65535 98
-8 9 string 7 111 128 2047 4660 65535 98 97
-8 10 string 8 111 128 2047 4660 65535 98 97 114
-8 Infinity string 8 111 128 2047 4660 65535 98 97 114
-8 NaN string 0 
-7 -Infinity string 0 
-7 -10 string 0 
-7 -9 string 0 
-7 -8 string 0 
-7 -7 string 0 
-7 -6 string 1 128
-7 -5 string 2 128 2047
-7 -7 string 0 
-7 -2 string 5 128 2047 4660 65535 98
-7 -1 string 6 128 2047 4660 65535 98 97
-7 0 string 0 
-7 0 string 0 
-7 1 string 0 
-7 2 string 0 
-7 3 string 0 
-7 4 string 1 128
-7 5 string 2 128 2047
-7 6 string 3 128 2047 4660
-7 7 string 4 128 2047 4660 65535
-7 8 string 5 128 2047 4660 65535 98
-7 9 string 6 128 2047 4660 65535 98 97
-7 10 string 7 128 2047 4660 65535 98 97 114
-7 Infinity string 7 128 2047 4660 65535 98 97 114
-7 NaN string 0 
-6 -Infinity string 0 
-6 -10 string 0 
-6 -9 string 0 
-6 -8 string 0 
-6 -7 string 0 
-6 -6 string 0 
-6 -5 string 1 2047
-6 -7 string 0 
-6 -2 string 4 2047 4660 65535 98
-6 -1 string 5 2047 4660 65535 98 97
-6 0 string 0 
-6 0 string 0 
-6 1 string 0 
-6 2 string 0 
-6 3 string 0 
-6 4 string 0 
-6 5 string 1 2047
-6 6 string 2 2047 4660
-6 7 string 3 2047 4660 65535
-6 8 string 4 2047 4660 65535 98
-6 9 string 5 2047 4660 65535 98 97
-6 10 string 6 2047 4660 65535 98 97 114
-6 Infinity string 6 2047 4660 65535 98 97 114
-6 NaN string 0 
-5 -Infinity string 0 
-5 -10 string 0 
-5 -9 string 0 
-5 -8 string 0 
-5 -7 string 0 
-5 -6 string 0 
-5 -5 string 0 
-5 -7 string 0 
-5 -2 string 3 4660 65535 98
-5 -1 string 4 4660 65535 98 97
-5 0 string 0 
-5 0 string 0 
-5 1 string 0 
-5 2 string 0 
-5 3 string 0 
-5 4 string 0 
-5 5 string 0 
-5 6 string 1 4660
-5 7 string 2 4660 65535
-5 8 string 3 4660 65535 98
-5 9 string 4 4660 65535 98 97
-5 10 string 5 4660 65535 98 97 114
-5 Infinity string 5 4660 65535 98 97 114
-5 NaN string 0 
-7 -Infinity string 0 
-7 -10 string 0 
-7 -9 string 0 
-7 -8 string 0 
-7 -7 string 0 
-7 -6 string 1 128
-7 -5 string 2 128 2047
-7 -7 string 0 
-7 -2 string 5 128 2047 4660 65535 98
-7 -1 string 6 128 2047 4660 65535 98 97
-7 0 string 0 
-7 0 string 0 
-7 1 string 0 
-7 2 string 0 
-7 3 string 0 
-7 4 string 1 128
-7 5 string 2 128 2047
-7 6 string 3 128 2047 4660
-7 7 string 4 128 2047 4660 65535
-7 8 string 5 128 2047 4660 65535 98
-7 9 string 6 128 2047 4660 65535 98 97
-7 10 string 7 128 2047 4660 65535 98 97 114
-7 Infinity string 7 128 2047 4660 65535 98 97 114
-7 NaN string 0 
-2 -Infinity string 0 
-2 -10 string 0 
-2 -9 string 0 
-2 -8 string 0 
-2 -7 string 0 
-2 -6 string 0 
-2 -5 string 0 
-2 -7 string 0 
-2 -2 string 0 
-2 -1 string 1 97
-2 0 string 0 
-2 0 string 0 
-2 1 string 0 
-2 2 string 0 
-2 3 string 0 
-2 4 string 0 
-2 5 string 0 
-2 6 string 0 
-2 7 string 0 
-2 8 string 0 
-2 9 string 1 97
-2 10 string 2 97 114
-2 Infinity string 2 97 114
-2 NaN string 0 
-1 -Infinity string 0 
-1 -10 string 0 
-1 -9 string 0 
-1 -8 string 0 
-1 -7 string 0 
-1 -6 string 0 
-1 -5 string 0 
-1 -7 string 0 
-1 -2 string 0 
-1 -1 string 0 
-1 0 string 0 
-1 0 string 0 
-1 1 string 0 
-1 2 string 0 
-1 3 string 0 
-1 4 string 0 
-1 5 string 0 
-1 6 string 0 
-1 7 string 0 
-1 8 string 0 
-1 9 string 0 
-1 10 string 1 114
-1 Infinity string 1 114
-1 NaN string 0 
0 -Infinity string 0 
0 -10 string 0 
0 -9 string 1 102
0 -8 string 2 102 111
0 -7 string 3 102 111 111
0 -6 string 4 102 111 111 128
0 -5 string 5 102 111 111 128 2047
0 -7 string 3 102 111 111
0 -2 string 8 102 111 111 128 2047 4660 65535 98
0 -1 string 9 102 111 111 128 2047 4660 65535 98 97
0 0 string 0 
0 0 string 0 
0 1 string 1 102
0 2 string 2 102 111
0 3 string 3 102 111 111
0 4 string 4 102 111 111 128
0 5 string 5 102 111 111 128 2047
0 6 string 6 102 111 111 128 2047 4660
0 7 string 7 102 111 111 128 2047 4660 65535
0 8 string 8 102 111 111 128 2047 4660 65535 98
0 9 string 9 102 111 111 128 2047 4660 65535 98 97
0 10 string 10 102 111 111 128 2047 4660 65535 98 97 114
0 Infinity string 10 102 111 111 128 2047 4660 65535 98 97 114
0 NaN string 0 
0 -Infinity string 0 
0 -10 string 0 
0 -9 string 1 102
0 -8 string 2 102 111
0 -7 string 3 102 111 111
0 -6 string 4 102 111 111 128
0 -5 string 5 102 111 111 128 2047
0 -7 string 3 102 111 111
0 -2 string 8 102 111 111 128 2047 4660 65535 98
0 -1 string 9 102 111 111 128 2047 4660 65535 98 97
0 0 string 0 
0 0 string 0 
0 1 string 1 102
0 2 string 2 102 111
0 3 string 3 102 111 111
0 4 string 4 102 111 111 128
0 5 string 5 102 111 111 128 2047
0 6 string 6 102 111 111 128 2047 4660
0 7 string 7 102 111 111 128 2047 4660 65535
0 8 string 8 102 111 111 128 2047 4660 65535 98
0 9 string 9 102 111 111 128 2047 4660 65535 98 97
0 10 string 10 102 111 111 128 2047 4660 65535 98 97 114
0 Infinity string 10 102 111 111 128 2047 4660 65535 98 97 114
0 NaN string 0 
1 -Infinity string 0 
1 -10 string 0 
1 -9 string 0 
1 -8 string 1 111
1 -7 string 2 111 111
1 -6 string 3 111 111 128
1 -5 string 4 111 111 128 2047
1 -7 string 2 111 111
1 -2 string 7 111 111 128 2047 4660 65535 98
1 -1 string 8 111 111 128 2047 4660 65535 98 97
1 0 string 0 
1 0 string 0 
1 1 string 0 
1 2 string 1 111
1 3 string 2 111 111
1 4 string 3 111 111 128
1 5 string 4 111 111 128 2047
1 6 string 5 111 111 128 2047 4660
1 7 string 6 111 111 128 2047 4660 65535
1 8 string 7 111 111 128 2047 4660 65535 98
1 9 string 8 111 111 128 2047 4660 65535 98 97
1 10 string 9 111 111 128 2047 4660 65535 98 97 114
1 Infinity string 9 111 111 128 2047 4660 65535 98 97 114
1 NaN string 0 
2 -Infinity string 0 
2 -10 string 0 
2 -9 string 0 
2 -8 string 0 
2 -7 string 1 111
2 -6 string 2 111 128
2 -5 string 3 111 128 2047
2 -7 string 1 111
2 -2 string 6 111 128 2047 4660 65535 98
2 -1 string 7 111 128 2047 4660 65535 98 97
2 0 string 0 
2 0 string 0 
2 1 string 0 
2 2 string 0 
2 3 string 1 111
2 4 string 2 111 128
2 5 string 3 111 128 2047
2 6 string 4 111 128 2047 4660
2 7 string 5 111 128 2047 4660 65535
2 8 string 6 111 128 2047 4660 65535 98
2 9 string 7 111 128 2047 4660 65535 98 97
2 10 string 8 111 128 2047 4660 65535 98 97 114
2 Infinity string 8 111 128 2047 4660 65535 98 97 114
2 NaN string 0 
3 -Infinity string 0 
3 -10 string 0 
3 -9 string 0 
3 -8 string 0 
3 -7 string 0 
3 -6 string 1 128
3 -5 string 2 128 2047
3 -7 string 0 
3 -2 string 5 128 2047 4660 65535 98
3 -1 string 6 128 2047 4660 65535 98 97
3 0 string 0 
3 0 string 0 
3 1 string 0 
3 2 string 0 
3 3 string 0 
3 4 string 1 128
3 5 string 2 128 2047
3 6 string 3 128 2047 4660
3 7 string 4 128 2047 4660 65535
3 8 string 5 128 2047 4660 65535 98
3 9 string 6 128 2047 4660 65535 98 97
3 10 string 7 128 2047 4660 65535 98 97 114
3 Infinity string 7 128 2047 4660 65535 98 97 114
3 NaN string 0 
4 -Infinity string 0 
4 -10 string 0 
4 -9 string 0 
4 -8 string 0 
4 -7 string 0 
4 -6 string 0 
4 -5 string 1 2047
4 -7 string 0 
4 -2 string 4 2047 4660 65535 98
4 -1 string 5 2047 4660 65535 98 97
4 0 string 0 
4 0 string 0 
4 1 string 0 
4 2 string 0 
4 3 string 0 
4 4 string 0 
4 5 string 1 2047
4 6 string 2 2047 4660
4 7 string 3 2047 4660 65535
4 8 string 4 2047 4660 65535 98
4 9 string 5 2047 4660 65535 98 97
4 10 string 6 2047 4660 65535 98 97 114
4 Infinity string 6 2047 4660 65535 98 97 114
4 NaN string 0 
5 -Infinity string 0 
5 -10 string 0 
5 -9 string 0 
5 -8 string 0 
5 -7 string 0 
5 -6 string 0 
5 -5 string 0 
5 -7 string 0 
5 -2 string 3 4660 65535 98
5 -1 string 4 4660 65535 98 97
5 0 string 0 
5 0 string 0 
5 1 string 0 
5 2 string 0 
5 3 string 0 
5 4 string 0 
5 5 string 0 
5 6 string 1 4660
5 7 string 2 4660 65535
5 8 string 3 4660 65535 98
5 9 string 4 4660 65535 98 97
5 10 string 5 4660 65535 98 97 114
5 Infinity string 5 4660 65535 98 97 114
5 NaN string 0 
6 -Infinity string 0 
6 -10 string 0 
6 -9 string 0 
6 -8 string 0 
6 -7 string 0 
6 -6 string 0 
6 -5 string 0 
6 -7 string 0 
6 -2 string 2 65535 98
6 -1 string 3 65535 98 97
6 0 string 0 
6 0 string 0 
6 1 string 0 
6 2 string 0 
6 3 string 0 
6 4 string 0 
6 5 string 0 
6 6 string 0 
6 7 string 1 65535
6 8 string 2 65535 98
6 9 string 3 65535 98 97
6 10 string 4 65535 98 97 114
6 Infinity string 4 65535 98 97 114
6 NaN string 0 
7 -Infinity string 0 
7 -10 string 0 
7 -9 string 0 
7 -8 string 0 
7 -7 string 0 
7 -6 string 0 
7 -5 string 0 
7 -7 string 0 
7 -2 string 1 98
7 -1 string 2 98 97
7 0 string 0 
7 0 string 0 
7 1 string 0 
7 2 string 0 
7 3 string 0 
7 4 string 0 
7 5 string 0 
7 6 string 0 
7 7 string 0 
7 8 string 1 98
7 9 string 2 98 97
7 10 string 3 98 97 114
7 Infinity string 3 98 97 114
7 NaN string 0 
8 -Infinity string 0 
8 -10 string 0 
8 -9 string 0 
8 -8 string 0 
8 -7 string 0 
8 -6 string 0 
8 -5 string 0 
8 -7 string 0 
8 -2 string 0 
8 -1 string 1 97
8 0 string 0 
8 0 string 0 
8 1 string 0 
8 2 string 0 
8 3 string 0 
8 4 string 0 
8 5 string 0 
8 6 string 0 
8 7 string 0 
8 8 string 0 
8 9 string 1 97
8 10 string 2 97 114
8 Infinity string 2 97 114
8 NaN string 0 
9 -Infinity string 0 
9 -10 string 0 
9 -9 string 0 
9 -8 string 0 
9 -7 string 0 
9 -6 string 0 
9 -5 string 0 
9 -7 string 0 
9 -2 string 0 
9 -1 string 0 
9 0 string 0 
9 0 string 0 
9 1 string 0 
9 2 string 0 
9 3 string 0 
9 4 string 0 
9 5 string 0 
9 6 string 0 
9 7 string 0 
9 8 string 0 
9 9 string 0 
9 10 string 1 114
9 Infinity string 1 114
9 NaN string 0 
10 -Infinity string 0 
10 -10 string 0 
10 -9 string 0 
10 -8 string 0 
10 -7 string 0 
10 -6 string 0 
10 -5 string 0 
10 -7 string 0 
10 -2 string 0 
10 -1 string 0 
10 0 string 0 
10 0 string 0 
10 1 string 0 
10 2 string 0 
10 3 string 0 
10 4 string 0 
10 5 string 0 
10 6 string 0 
10 7 string 0 
10 8 string 0 
10 9 string 0 
10 10 string 0 
10 Infinity string 0 
10 NaN string 0 
Infinity -Infinity string 0 
Infinity -10 string 0 
Infinity -9 string 0 
Infinity -8 string 0 
Infinity -7 string 0 
Infinity -6 string 0 
Infinity -5 string 0 
Infinity -7 string 0 
Infinity -2 string 0 
Infinity -1 string 0 
Infinity 0 string 0 
Infinity 0 string 0 
Infinity 1 string 0 
Infinity 2 string 0 
Infinity 3 string 0 
Infinity 4 string 0 
Infinity 5 string 0 
Infinity 6 string 0 
Infinity 7 string 0 
Infinity 8 string 0 
Infinity 9 string 0 
Infinity 10 string 0 
Infinity Infinity string 0 
Infinity NaN string 0 
NaN -Infinity string 0 
NaN -10 string 0 
NaN -9 string 1 102
NaN -8 string 2 102 111
NaN -7 string 3 102 111 111
NaN -6 string 4 102 111 111 128
NaN -5 string 5 102 111 111 128 2047
NaN -7 string 3 102 111 111
NaN -2 string 8 102 111 111 128 2047 4660 65535 98
NaN -1 string 9 102 111 111 128 2047 4660 65535 98 97
NaN 0 string 0 
NaN 0 string 0 
NaN 1 string 1 102
NaN 2 string 2 102 111
NaN 3 string 3 102 111 111
NaN 4 string 4 102 111 111 128
NaN 5 string 5 102 111 111 128 2047
NaN 6 string 6 102 111 111 128 2047 4660
NaN 7 string 7 102 111 111 128 2047 4660 65535
NaN 8 string 8 102 111 111 128 2047 4660 65535 98
NaN 9 string 9 102 111 111 128 2047 4660 65535 98 97
NaN 10 string 10 102 111 111 128 2047 4660 65535 98 97 114
NaN Infinity string 10 102 111 111 128 2047 4660 65535 98 97 114
NaN NaN string 0 
===*/

print('non-bmp');

function nonBmpTest() {
    var str = 'foo\u0080\u07ff\u1234\uffffbar';

    var numbers = [
        Number.NEGATIVE_INFINITY, -10, -9, -8, -7, -6, -5, -4 -3, -2, -1,
        -0, +0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, Number.POSITIVE_INFINITY, Number.NaN
    ];
    var t;

    function test(s, e) {
        var x = str.slice(s, e);
        var tmp = [];
        var i;

        for (i = 0; i < x.length; i++) {
            tmp.push(x.charCodeAt(i));
        }

        print(s, e, typeof x, x.length, tmp.join(' '));
    }

    for (i = 0; i < numbers.length; i++) {
        for (j = 0; j < numbers.length; j++) {
            test(numbers[i], numbers[j]);
        }
    }
}

try {
    nonBmpTest();
} catch (e) {
    print(e);
}

/*===
coercion
TypeError
TypeError
boolean string 2 ue
boolean string 2 ls
number string 2 34
string string 2 cd
object string 2 2,
object string 2 bj
===*/

print('coercion');

function coercionTest() {
    function test(str, s, e) {
        try {
            t = String.prototype.slice.call(str, s, e);
            print(typeof str, typeof t, t.length, t);
        } catch (e) {
            print(e.name);
        }
    }

    test(undefined, 2, 4);
    test(null, 2, 4);
    test(true, 2, 4);
    test(false, 2, 4);
    test(12345, 2, 4);
    test('abcdef', 2, 4);
    test([1,2,3], 2, 4);
    test({ foo: 1, bar: 1 }, 2, 4);
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
