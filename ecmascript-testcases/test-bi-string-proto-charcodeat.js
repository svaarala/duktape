/*===
basic test
97
98
99
65
66
67
0
102
111
111
129
98
97
114
2047
113
117
117
120
4660
98
97
122
65244
120
121
122
65535
===*/

print('basic test');

function charCodeAtTest() {
    var str = new String('abcABC\u0000foo\u0081bar\u07ffquux\u1234baz\ufedcxyz\uffff');
    var i;

    for (i = 0; i < str.length; i++) {
        print(str.charCodeAt(i));
    }
}

try {
    charCodeAtTest();
} catch (e) {
    print(e);
}

/*===
oob test
-3 NaN
-2 NaN
-1 NaN
0 98
1 97
2 114
3 NaN
4 NaN
5 NaN
-Infinity NaN
Infinity NaN
NaN 98
===*/

print('oob test');

function outOfBoundsTest() {
    var str = new String('bar');
    var i;

    function pc(idx) {
        var c = str.charCodeAt(idx);
        print(idx, c);
    }

    for (i = -3; i < 6; i++) {
        pc(i);
    }

    pc(Number.NEGATIVE_INFINITY),
    pc(Number.POSITIVE_INFINITY),

    // ToInteger(NaN) coerces to 0 -> 'b'
    pc(Number.NaN);
}

try {
    outOfBoundsTest();
} catch (e) {
    print(e);
}

/*===
random access test
random access test done
===*/

/* Random access test for a large string.  The purpose of this test is to
 * stress the stringcache.  Note that the test is not deterministic.
 */

print('random access test');

function randomAccessTest() {
    var tmp = [];
    var str;
    var i;
    var idx, c;

    for (i = 0; i < 65536; i++) {
        tmp.push(String.fromCharCode(i));
    }
    str = new String(tmp.join(''));

    for (i = 0; i < 100000; i++) {
        idx = Math.floor(Math.random() * 65536);
        c = str.charCodeAt(idx);
        if (c !== idx) {
            print('random access test failed for index', idx);
        }
    }

    print('random access test done');
}

try {
    randomAccessTest();
} catch (e) {
    print(e);
}

/*===
coercion
testing undefined
TypeError
testing null
TypeError
testing true
0 number 116
1 number 114
2 number 117
3 number 101
testing false
0 number 102
1 number 97
2 number 108
3 number 115
4 number 101
testing 123
0 number 49
1 number 50
2 number 51
testing foo
0 number 102
1 number 111
2 number 111
testing 1,2
0 number 49
1 number 44
2 number 50
testing [object Object]
0 number 91
1 number 111
2 number 98
3 number 106
4 number 101
5 number 99
6 number 116
7 number 32
8 number 79
9 number 98
10 number 106
11 number 101
12 number 99
13 number 116
14 number 93
===*/

/* charCodeAt() is generic, check 'this' coercion */

print('coercion');

function thisCoercionTest() {
    function test(x) {
        var c;
        var i;

        print('testing', x);
        try {
            for (i = 0; ; i++) {
                c = String.prototype.charCodeAt.call(x, i);
                if (isNaN(c)) {
                    break;
                }
                print(i, typeof c, c);
            }
        } catch (e) {
            print(e.name);
        }
    }

    // undefined and null cause TypeError from CheckObjectCoercible

    test(undefined);
    test(null);

    test(true);
    test(false);
    test(123);
    test('foo');
    test([1,2]);
    test({ foo: 1, bar: 2 });
}

try {
    thisCoercionTest();
} catch (e) {
    print(e);
}
