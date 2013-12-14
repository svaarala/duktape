/*
 *  Arguments binding cannot be directly accessed from an inner function as
 *  there is already some binding in the inner function which prevents the
 *  access.  However, it can be accessed indirectly.
 */

/*===
foo bar quux
===*/

function f() {
    var foo = arguments;
    function g() {
        print(foo[0], foo[1], foo[2]);
    }
    return g;
}

try {
    t = f('foo', 'bar', 'quux');
    t();
} catch (e) {
    print(e);
}
