/*
 *  Example from compiler.rst.
 */

/*===
321
123
===*/

var foo = 123;
var myfunc;

function f(x) {
    eval(x);

    return function () { print(foo); }
}

// declare 'foo' in f(), returned closure sees this 'foo' instead
// of the global one

myfunc = f('var foo = 321');
myfunc();  // prints 321, not 123

// don't declare 'foo' in f(), returned closure sees the global 'foo'
// instead of the global one

myfunc = f('var quux = 432');
myfunc();  // prints 123
