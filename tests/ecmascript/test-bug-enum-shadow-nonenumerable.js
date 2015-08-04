/*
 *  Issue revealed by test262 ch12/12.6/12.6.4/12.6.4-2:
 *
 *  Enumeration issue: property P is enumerated by Duktape in this situation:
 *
 *      PARENT <-- contains P as enumerable property
 *        ^
 *        | internal prototype
 *        |
 *      OBJECT <-- contains P as non-enumerable property
 *
 *  Not sure what E5/E5.1 specification says about this (it is somewhat vague
 *  for strict enumeration rules) but test262 requires that in the above case
 *  P must not be enumerated.
 *
 *  Neither V8 nor Rhino (some versions at least) pass this test.
 */

/*===
test1
foo
bar
y enums "prop": false
test2
child enums "prop": false
===*/

/* Simple object case */
function test1() {
    var x = {};
    var y = Object.create(x);

    Object.defineProperty(x, 'prop', {
        value: 'foo', writable: true, enumerable: true, configurable: false
    });

    Object.defineProperty(y, 'prop', {
        value: 'bar', writable: true, enumerable: false, configurable: false
    });

    print(x.prop);
    print(y.prop);

    var hasProp = false;
    for (var name in y) {
        if (name === 'prop') { hasProp = true; }
    }
    print('y enums "prop":', hasProp);
}

/* Function case */
function test2() {
    var proto = { prop: 'foo' };
    var constructor = function () {};
    constructor.prototype = proto;

    var child = new constructor();
    Object.defineProperty(child, 'prop', {
        value: 'bar', writable: true, enumerable: false, configurable: false
    });

    var hasProp = false;
    for (var name in child) {
        if (name === 'prop') { hasProp = true; }
    }
    print('child enums "prop":', hasProp);
}

print('test1');
try {
    test1();
} catch (e) {
    print(e);
}

print('test2');
try {
    test2();
} catch (e) {
    print(e);
}
