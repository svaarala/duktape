/*
 *  Test() should have the same behavior as exec(), except that the return
 *  value is just a boolean.
 */

var r, t;

/*===
true
false
===*/

try {
    r = /foo/;
    t = r.test('foo');
    print(t);
    t = r.test('bar');
    print(t);
} catch (e) {
    print(e.name);
}

/*===
true
false
===*/

try {
    r = /(foo)/i;
    t = r.test('Foo');
    print(t);
    t = r.test('bar');
    print(t);
} catch (e) {
    print(e.name);
}

/*===
0
true 3
true 6
true 9
false 0
===*/

try {
    r = /foo/gi;
    print(r.lastIndex);
    t = r.test('fooFooFOO');
    print(t, r.lastIndex);
    t = r.test('fooFooFOO');
    print(t, r.lastIndex);
    t = r.test('fooFooFOO');
    print(t, r.lastIndex);
    t = r.test('fooFooFOO');
    print(t, r.lastIndex);
} catch (e) {
    print(e.name);
}
