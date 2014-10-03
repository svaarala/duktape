/*
 *  A bound function must have a 'caller' and an 'arguments' property mapped
 *  to the same thrower object.  At some point 'caller' and 'arguments' were
 *  missing.
 */

/*===
length true
prototype true
length true
caller true
arguments true
===*/

function test(obj, prop) {
    print(prop, prop in obj);
}

try {
    f = function() {};
    test(f, 'length');
    test(f, 'prototype');

    // bound function does not have a 'prototype'
    g = f.bind(null);
    test(g, 'length');
    test(g, 'caller');
    test(g, 'arguments');
} catch (e) {
    print(e);
}
