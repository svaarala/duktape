/*
 *  Constructor call (new) for a bound constructor.
 */

/*===
object
true
false
this: object undefined
object
foo,bar,quux,baz,quuux
===*/

function MyConstructor(a, b, c, d, e) {
    print('this:', typeof this, this.foo);
    this.list = [ a, b, c, d, e ];
}

function test() {
    var bound;
    var t;

    // Native constructor function, bound argument.  The 'this' binding
    // is ignored.
    bound = RegExp.bind({ foo: 123, ignored: true }, '^(foo){2}$');
    t = new bound('i');
    print(typeof t);
    print(t.test('fooFOO'));
    print(t.test('foobar'));

    // ECMAScript constructor function, bound arguments.
    bound = MyConstructor.bind({ foo: 123, ignored: true }, 'foo', 'bar', 'quux');
    t = new bound('baz', 'quuux');
    print(typeof t);
    print(t.list);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
