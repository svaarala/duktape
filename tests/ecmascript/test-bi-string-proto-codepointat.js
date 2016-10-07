/*
 *  String.prototype.codePointAt()
 */

/*===
-3 NaN undefined
-2 NaN undefined
-1 NaN undefined
0 102 102
1 111 111
2 111 111
3 51966 51966
4 55357 128169
5 56489 56489
6 48879 48879
7 98 98
8 97 97
9 114 114
10 55357 55357
11 55357 55357
12 55357 55357
13 120 120
14 56489 56489
15 55357 55357
16 98 98
17 97 97
18 122 122
19 55357 55357
20 NaN undefined
21 NaN undefined
22 NaN undefined
[object Object]
-3 NaN undefined
-2 NaN undefined
-1 NaN undefined
0 91 91
1 111 111
2 98 98
3 106 106
4 101 101
5 99 99
6 116 116
7 32 32
8 79 79
9 98 98
10 106 106
11 101 101
12 99 99
13 116 116
14 93 93
15 NaN undefined
16 NaN undefined
17 NaN undefined
18 NaN undefined
19 NaN undefined
===*/

function test() {
    var x = 'foo' +
            '\ucafe' +  // simple non-surrogate
            '\uD83D\uDCA9' +  // valid U+1F4A9 in surrogate pair encoding
            '\ubeef' +  // simple non-surrogate
            'bar' +
            '\ud83d\ud83d' +  // invalid surrogate pair
            '\ud83dx' +       // -""-
            '\udca9\ud83d' +  // -""-
            'baz' +
            '\ud83d';         // invalid surrogate pair, string ends before pair
    for (i = -3; i < x.length + 3; i++) {
        print(i, x.charCodeAt(i), x.codePointAt(i));
    }

    // .codePointAt() is generic like .charCodeAt(), but they simply String() coerce
    // their argument first so this is coerced to [object Object] and the char lookup
    // is then applied.  Codepoints won't be looked up from the object properties.
    var obj = { 0: 'f', 1: 'o', 2: 'o',
                3: '\ud83d', 4: '\udca9',
                5: 'b', 6: 'a', 7: 'r', length: 8 };
    print(String(obj));

    for (i = -3; i < 20; i++) {
        print(i, String.prototype.charCodeAt.call(obj, i), String.prototype.codePointAt.call(obj, i));
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
