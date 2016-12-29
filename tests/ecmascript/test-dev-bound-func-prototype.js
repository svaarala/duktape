/*
 *  Bound function internal prototype is copied from the target in ES2015.
 *  In ES5 it is always Function.prototype.  Test for ES2015 behavior.
 */

/*===
bar
bar
true
quux
bar
false
===*/

function test() {
    var F = function foo() {};
    var proto = { foo: 'bar' };
    Object.setPrototypeOf(F, proto);

    // Create a bound function; its prototype is copied from the current
    // prototype of the target.
    var G = Function.prototype.bind.call(F);
    print(F.foo);
    print(G.foo);
    print(Object.getPrototypeOf(G) === proto);

    // Alter target's internal prototype; doesn't affect the bound function.
    proto = { foo: 'quux' };
    Object.setPrototypeOf(F, proto);
    print(F.foo);
    print(G.foo);
    print(Object.getPrototypeOf(G) === proto);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
