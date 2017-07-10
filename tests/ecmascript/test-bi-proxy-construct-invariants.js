/*
 *  Proxy 'construct' trap return value invariant: return value must be an
 *  Object or a TypeError is thrown.
 */

/*===
bar-trap
TypeError
===*/

function test() {
    var proxy;

    function MyConstructor(a, b, c) {
        this.foo = 'bar';
    }

    // Proxy 'construct' must return an object.
    proxy = new Proxy(MyConstructor, {
        construct: function (targ, argArray, newTarget) {
            return { foo: 'bar-trap' };
        }
    });
    try {
        print(new proxy().foo);
    } catch (e) {
        print(e.stack || e);
    }

    // If the return value is not an object, TypeError.
    proxy = new Proxy(MyConstructor, {
        construct: function (targ, argArray, newTarget) {
            return 'xyz';
        }
    });
    try {
        print(new proxy());
    } catch (e) {
        print(e.name);
        //print(e.stack || e);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
