/*
 *  Misc tests related to Reflect.construct() and other special call handling.
 */

/*===
TypeError: [object Function] not constructable
TypeError: [object Function] not constructable
TypeError: [object Function] not constructable
TypeError: [object Function] not constructable
TypeError: undefined not callable
TypeError: [object Function] not constructable
TypeError: [object Function] not constructable
TypeError: [object Function] not constructable
===*/

function test() {
    function foo() {
        print('foo called');
    }

    // Reflect.construct for foo.call() fails because Function.prototype.call()
    // is not constructable.
    try {
        print(Reflect.construct(foo.call, [ 1, 2, 3 ]));
    } catch (e) {
        print(e);
    }

    // Same test, argArray is an ECMAScript function (this makes no sense,
    // but covers a development time bug).
    try {
        print(Reflect.construct(foo.call, foo));
    } catch (e) {
        print(e);
    }

    // Same for foo.apply().
    try {
        print(Reflect.construct(foo.apply, [ 1, 2, 3 ]));
    } catch (e) {
        print(e);
    }

    // Reflect.construct(), non-constructable target and missing argArray.
    // Should fail in the non-constructable check before considering
    // argArray.
    try {
        print(Reflect.construct(foo.call));
    } catch (e) {
        print(e);
    }

    // Reflect.construct() without arguments (covers a specific internal path):
    try {
        print(Reflect.construct());
    } catch (e) {
        print(e);
    }

    // new Reflect.construct() fails, not constructable.
    try {
        print(new Reflect.construct(foo, [ 1, 2, 3 ]));
    } catch (e) {
        print(e);
    }

    // Same for foo.call() and foo.apply().
    try {
        print(new foo.call());
    } catch (e) {
        print(e);
    }
    try {
        print(new foo.apply());
    } catch (e) {
        print(e);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
