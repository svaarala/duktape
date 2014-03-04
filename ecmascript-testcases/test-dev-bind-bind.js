/*
 *  Test that bind() can also be used with bind().
 */

/*===
foo this: [object global] args: foo bar quux baz
foo this: mythis args: a b c d
foo this: mythis args: a b c final
===*/

function foo(x,y,z,w) {
    print('foo', 'this:', this, 'args:', x, y, z, w);
}

function test() {
    var f, g, h;

    // plain call
    foo('foo', 'bar', 'quux', 'baz');

    // bound call
    f = foo.bind('mythis', 'a', 'b', 'c', 'd');
    f();

    // bind bind() so that when the result is called, the resulting
    // bind call arguments look like the "f = foo.bind" above
    g = Function.prototype.bind.bind(foo, 'mythis', 'a', 'b', 'c');  // leave last arg unbound
    h = g('final');  // give last arg
    h();
}

try {
    test();
} catch (e) {
    print(e);
}
