/*
 *  Simple tests for Duktape.Pointer and how this custom type behaves
 *  in coercions etc.  More detailed tests separately.
 */

function dumpClass(x) {
    return Object.prototype.toString.call(x);
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
    print('test pointer 1');
    testPointer1();
    print('test pointer 2');
    testPointer2();
} catch (e) {
    print(e);
}
