/*===
-1 NaN undefined
0 102 102
1 111 111
2 111 111
3 55357 128169
4 56489 56489
5 98 98
6 97 97
7 114 114
8 NaN undefined
8 NaN undefined
7 114 114
6 97 97
5 98 98
4 56489 56489
3 55357 128169
2 111 111
1 111 111
0 102 102
-1 NaN undefined
-1 NaN undefined
0 102 102
1 111 111
2 111 111
3 55357 128169
4 56489 56489
5 98 98
6 97 97
7 114 114
8 120 120
9 120 120
10 120 120
11 120 120
12 120 120
13 120 120
14 120 120
15 120 120
16 120 120
17 120 120
18 120 120
19 120 120
20 120 120
20 120 120
19 120 120
18 120 120
17 120 120
16 120 120
15 120 120
14 120 120
13 120 120
12 120 120
11 120 120
10 120 120
9 120 120
8 120 120
7 114 114
6 97 97
5 98 98
4 56489 56489
3 55357 128169
2 111 111
1 111 111
0 102 102
-1 NaN undefined
===*/

function testOne(s, startIdx, endIdx) {
    for (var i = startIdx; i <= endIdx; i++) {
        print(i, s.charCodeAt(i), s.codePointAt(i));
    }
    for (var i = endIdx; i >= startIdx; i--) {
        print(i, s.charCodeAt(i), s.codePointAt(i));
    }
}

function test() {
    var str = 'foo\u{1f4a9}bar';
    testOne(str, -1, str.length);

    var str = 'foo\u{1f4a9}bar' + ('x').repeat(1000);
    testOne(str, -1, 20);
}

test();
