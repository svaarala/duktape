/*
 *  Simple tests for Duktape.Buffer and Duktape.Pointer and how these two
 *  custom types behave in coercions etc.  More detailed tests separately.
 */

function dumpClass(x) {
    return Object.prototype.toString.call(x);
}

/*===
test buffer 1
buffer [object Buffer]
buffer [object Buffer]
object [object Buffer]
true
string
buffer
string
buffer
foo
===*/

function testBuffer1() {
    var plain1 = Duktape.dec('hex', 'deadbeef');
    var plain2 = Duktape.Buffer('foo');
    var object1 = new Duktape.Buffer('bar');

    print(typeof plain1, dumpClass(plain1));
    print(typeof plain2, dumpClass(plain2));
    print(typeof object1, dumpClass(object1));
    print(object1 instanceof Duktape.Buffer);

    print(typeof object1.toString());
    print(typeof object1.valueOf());

    // method lookup through prototype when base value is plain
    print(typeof plain1.toString());
    print(typeof plain1.valueOf());
    print(plain2.toString());
}

/*===
test buffer 2
0 0 F F
0 1 F F
0 2 F F
0 3 F F
0 4 F F
0 5 F F
0 6 F F
0 7 F F
0 8 F F
1 0 F F
1 1 F F
1 2 F F
1 3 F F
1 4 F F
1 5 F F
1 6 F F
1 7 F F
1 8 F F
2 0 F F
2 1 F F
2 2 F F
2 3 F F
2 4 F F
2 5 T F
2 6 F F
2 7 F F
2 8 F F
3 0 T F
3 1 F F
3 2 F F
3 3 F F
3 4 T F
3 5 F F
3 6 T F
3 7 F F
3 8 F F
4 0 T F
4 1 F F
4 2 F F
4 3 F F
4 4 T F
4 5 F F
4 6 T F
4 7 F F
4 8 F F
5 0 F F
5 1 F F
5 2 F F
5 3 T F
5 4 F F
5 5 F F
5 6 F F
5 7 F F
5 8 F F
6 0 T F
6 1 F F
6 2 F F
6 3 F F
6 4 T F
6 5 F F
6 6 T F
6 7 F F
6 8 F F
7 0 F F
7 1 F F
7 2 F F
7 3 T F
7 4 F F
7 5 F F
7 6 F F
7 7 F F
7 8 F F
8 0 F F
8 1 T F
8 2 T F
8 3 F F
8 4 F F
8 5 F F
8 6 F F
8 7 T F
8 8 T F
9 0 F F
9 1 T F
9 2 T F
9 3 F F
9 4 F F
9 5 F F
9 6 F F
9 7 T F
9 8 T F
10 0 F F
10 1 T F
10 2 T F
10 3 F F
10 4 F F
10 5 F F
10 6 F F
10 7 T F
10 8 T F
11 0 F F
11 1 T F
11 2 T F
11 3 F F
11 4 F F
11 5 F F
11 6 F F
11 7 F F
11 8 F F
===*/

function testBuffer2() {
    var B = Duktape.Buffer;
    var values1 = [
        undefined, null, true, false,
        0, 123, Number(0), Number(123),
        'foo', String('foo'), B('foo'), new B('foo')
    ];
    var values2 = [
        B(''), B('foo'), B('foo'), B('123'),
        B('0'), B('1'), new B(''), new B('foo'),
        new B('foo')
    ];
    var i, j;
    var T = { true: 'T', false: 'F' };
    for (i = 0; i < values1.length; i++) {
        for (j = 0; j < values2.length; j++) {
            var v1 = values1[i], v2 = values2[j];
            var eq1 = (v1 == v2), eq2 = (v2 == v1);
            var seq1 = (v1 === v2), seq2 = (v2 === v1);
            if (eq1 != eq2 || seq1 != seq2) {
                throw new Error("equality or strict equality not symmetric");
            }
            print(i, j, T[eq1], T[seq1]);
        }
    }
}

/*===
test pointer 1
pointer [object Pointer]
object [object Pointer]
true
string
pointer
string
pointer
0 1
0
3
===*/

function testPointer1() {
    // XXX: these yield random heap pointers now, maybe changes later
    var plain1 = Duktape.Pointer('foo');
    var object1 = new Duktape.Pointer('bar');

    print(typeof plain1, dumpClass(plain1));
    print(typeof object1, dumpClass(object1));
    print(object1 instanceof Duktape.Pointer);

    print(typeof object1.toString());
    print(typeof object1.valueOf());

    // method lookup through prototype when base value is plain
    print(typeof plain1.toString());
    print(typeof plain1.valueOf());

    var ptr_null = Duktape.Pointer();
    var ptr_nonnull = Duktape.Pointer('foo');

    // unary plus coercion is ToNumber()
    print(+ptr_null, +ptr_nonnull);

    // addition coerces with ToNumber() which results in 0 for NULL
    // and 1 for non-NULL pointer now
    print(ptr_null + ptr_null + ptr_null);
    print(ptr_nonnull + ptr_nonnull + ptr_null + ptr_nonnull);
}

/*===
test pointer 2
0 0 F F
0 1 F F
1 0 F F
1 1 F F
2 0 F F
2 1 F F
3 0 F F
3 1 F F
4 0 F F
4 1 F F
5 0 F F
5 1 F F
6 0 F F
6 1 F F
7 0 F F
7 1 F F
8 0 F F
8 1 F F
9 0 F F
9 1 F F
10 0 F F
10 1 F F
11 0 F F
11 1 F F
12 0 F F
12 1 F F
13 0 F F
13 1 F F
14 0 F F
14 1 F F
15 0 F F
15 1 F F
16 0 F F
16 1 F F
17 0 F F
17 1 F F
18 0 T T
18 1 F F
19 0 F F
19 1 T T
===*/

function testPointer2() {
    var P = Duktape.Pointer;
    var p1 = P();  // NULL
    var p2 = P('foo');  // non-NULL
    var values1 = [
        undefined, null, true, false,
        0, 1, 123, Number(0),
        Number(1), Number(123), '', '0',
        '1', 'foo', String(''), String('0'),
        String('1'), String('foo'), p1, p2
    ];
    var values2 = [ p1, p2 ];
    var i, j;
    var T = { true: 'T', false: 'F' };
    for (i = 0; i < values1.length; i++) {
        for (j = 0; j < values2.length; j++) {
            var v1 = values1[i], v2 = values2[j];
            var eq1 = (v1 == v2), eq2 = (v2 == v1);
            var seq1 = (v1 === v2), seq2 = (v2 === v1);
            if (eq1 != eq2 || seq1 != seq2) {
                throw new Error("equality or strict equality not symmetric");
            }
            print(i, j, T[eq1], T[seq1]);
        }
    }
}

try {
    print('test buffer 1');
    testBuffer1();
    print('test buffer 2');
    testBuffer2();
} catch (e) {
    print(e);
}

try {
    print('test pointer 1');
    testPointer1();
    print('test pointer 2');
    testPointer2();
} catch (e) {
    print(e);
}
