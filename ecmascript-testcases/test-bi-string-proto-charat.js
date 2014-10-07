/*===
basic test
string 1 a
string 1 b
string 1 c
string 1 A
string 1 B
string 1 C
string 1 U+0000
string 1 f
string 1 o
string 1 o
string 1 U+0081
string 1 b
string 1 a
string 1 r
string 1 U+07FF
string 1 q
string 1 u
string 1 u
string 1 x
string 1 U+1234
string 1 b
string 1 a
string 1 z
string 1 U+FEDC
string 1 x
string 1 y
string 1 z
string 1 U+FFFF
===*/

print('basic test');

function charAtTest() {
    var str = new String('abcABC\u0000foo\u0081bar\u07ffquux\u1234baz\ufedcxyz\uffff');
    var i;

    // we want to print out ascii but don't want to rely on charCodeAt().
    // use a hashmap to "stringify" the non-ascii chars
    var map = {
        '\u0000': 'U+0000',
        '\u0081': 'U+0081',
        '\u07ff': 'U+07FF',
        '\u1234': 'U+1234',
        '\ufedc': 'U+FEDC',
        '\uffff': 'U+FFFF',
    }

    function pc(c) {
        print(typeof c, c.length, (c >= ' ' && c <= '~' ? c : map[c]));
    }

    // print in order
    for (i = 0; i < str.length; i++) {
        pc(str.charAt(i));
    }
}

try {
    charAtTest();
} catch (e) {
    print(e);
}

/*===
oob test
-3 0 
-2 0 
-1 0 
0 1 b
1 1 a
2 1 r
3 0 
4 0 
5 0 
-Infinity 0 
Infinity 0 
NaN 1 b
===*/

print('oob test');

function outOfBoundsTest() {
    var str = new String('bar');
    var i;

    function pc(idx) {
        var c = str.charAt(idx);
        print(idx, c.length, c);
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
    var idx, c1, c2;

    for (i = 0; i < 65536; i++) {
        tmp.push(String.fromCharCode(i));
    }
    str = new String(tmp.join(''));

    for (i = 0; i < 100000; i++) {
        idx = Math.floor(Math.random() * 65536);
        c1 = str.charAt(idx);
        c2 = String.fromCharCode(idx);  // XXX: unfortunately we depend on this too
        if (c1 !== c2) {
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
0 string t
1 string r
2 string u
3 string e
testing false
0 string f
1 string a
2 string l
3 string s
4 string e
testing 123
0 string 1
1 string 2
2 string 3
testing foo
0 string f
1 string o
2 string o
testing 1,2
0 string 1
1 string ,
2 string 2
testing [object Object]
0 string [
1 string o
2 string b
3 string j
4 string e
5 string c
6 string t
7 string  
8 string O
9 string b
10 string j
11 string e
12 string c
13 string t
14 string ]
===*/

/* charAt() is generic, check 'this' coercion */

print('coercion');

function thisCoercionTest() {
    function test(x) {
        var c;
        var i;

        print('testing', x);
        try {
            for (i = 0; ; i++) {
                c = String.prototype.charAt.call(x, i);
                if (c === '') {
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
