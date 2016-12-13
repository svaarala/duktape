/*
 *  Function constructor
 */

/*===
constructor
true true
false false false
true 1
false false true
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
    var pd;

    /* Properties. */

    print('prototype' in Function, Function.prototype === Function.prototype);
    pd = Object.getOwnPropertyDescriptor(Function, 'prototype');
    print(pd.writable, pd.enumerable, pd.configurable);
    print('length' in Function, Function.length);
    pd = Object.getOwnPropertyDescriptor(Function, 'length');
    print(pd.writable, pd.enumerable, pd.configurable);

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
