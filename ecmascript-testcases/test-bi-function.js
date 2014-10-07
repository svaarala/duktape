/*
 *  Function objects (E5 Section 15.3).
 */

/* XXX: toString(), call(), apply() */
/* XXX: property attributes */

/*===
constructor
true true
true 1
no args
no args
one arg foo
one arg foo
two args foo bar
two args foo bar
seven args foo bar quux undefined undefined undefined undefined
seven args foo bar quux undefined undefined undefined undefined
foobarquux
foobarquux
foobarquux
===*/

function functionConstructorTest() {
    var f;

    /* Properties. */

    print('prototype' in Function, Function.prototype === Function.prototype);
    print('length' in Function, Function.length);

    /* Test Function() called both as a function and as a constructor. */

    f = Function('print("no args");');
    f('foo', 'bar', 'quux');
    f = new Function('print("no args");');
    f('foo', 'bar', 'quux');

    f = Function('x', 'print("one arg", x);');
    f('foo', 'bar', 'quux');
    f = new Function('x', 'print("one arg", x);');
    f('foo', 'bar', 'quux');

    f = Function('x', 'y', 'print("two args", x, y);');
    f('foo', 'bar', 'quux');
    f = new Function('x', 'y', 'print("two args", x, y);');
    f('foo', 'bar', 'quux');

    f = Function('a', 'b', 'c', 'd', 'e', 'f', 'g', 'print("seven args", a, b, c, d, e, f, g);');
    f('foo', 'bar', 'quux');
    f = new Function('a', 'b', 'c', 'd', 'e', 'f', 'g', 'print("seven args", a, b, c, d, e, f, g);');
    f('foo', 'bar', 'quux');

    /* Example from specification (E5.1, Section 15.3.2.1, NOTE at bottom). */

    f = new Function("a", "b", "c", "return a+b+c");
    print(f('foo', 'bar', 'quux', 'baz'));
    f = new Function("a, b, c", "return a+b+c");
    print(f('foo', 'bar', 'quux', 'baz'));
    f = new Function("a,b", "c", "return a+b+c");
    print(f('foo', 'bar', 'quux', 'baz'));
}

try {
    print('constructor');
    functionConstructorTest();
} catch (e) {
    print(e.stack || e);
}

/*===
prototype
[object Function]
true
true
true 0
true true
undefined undefined
===*/

function functionPrototypeTest() {
    var ret;

    print(Object.prototype.toString.call(Function.prototype));
    print(Object.getPrototypeOf(Function.prototype) === Object.prototype);
    print(Object.isExtensible(Function.prototype));
    print('length' in Function.prototype, Function.prototype.length);
    print('constructor' in Function.prototype, Function.prototype.constructor === Function);

    ret = Function.prototype('foo', 'bar');
    print(typeof ret, ret);
}

try {
    print('prototype');
    functionPrototypeTest();
} catch (e) {
    print(e.stack || e);
}

/*===
instance
true true
true true
true 3
===*/

function functionInstanceTest() {
    var f = function test(a, b, c) {};

    print('prototype' in f, typeof f.prototype === 'object');
    print('constructor' in f.prototype, f.prototype.constructor === f);
    print('length' in f, f.length);
}

try {
    print('instance');
    functionInstanceTest();
} catch (e) {
    print(e.stack || e);
}
