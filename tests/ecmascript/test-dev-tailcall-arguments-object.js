/*
 *  Arguments object for tailcalls.
 */

/*===
object
3
foo
bar
quux
undefined
undefined
undefined
===*/

function foo() {
    var args = arguments;
    print(typeof arguments);
    print(arguments.length);
    print(arguments[0]);
    print(arguments[1]);
    print(arguments[2]);
    print(arguments[3]);
    print(arguments[4]);
    print(arguments[5]);
}

function bar(a,b,c,d) {
    return foo(a,b,c);
}

function test() {
    bar('foo', 'bar', 'quux', 'baz', 'quuux', 'baz');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
