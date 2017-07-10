/*
 *  Proxy (ES2015) 'construct'.
 */

/*===
- target not callable
object
[object Object]
TypeError
- target not constructable
function
[object Function]
TypeError
- proxy constructable but trap not callable
function
[object Function]
TypeError
- proxy constructable but trap only callable
function
[object Function]
0
- basic case
function
[object Function]
construct trap
true
true
object 2 123 234 undefined
true
bar-trap
- Reflect.construct
construct trap
true
true
object 2 123 234 undefined
true
bar-trap
- passthrough
function
[object Function]
MyConstructor called
object [object Object]
function
123 234 undefined undefined undefined
bar
===*/

var globalProxyRef;

function MyConstructor(a, b, c, d, e) {
    print('MyConstructor called');
    print(typeof this, this);
    print(typeof new.target);  // exact new.target behavior covered by separate test(s)
    print(a, b, c, d, e);
    this.foo = 'bar';
}

function test() {
    var target;
    var handler;
    var proxy;

    // If the (initial) target is not callable, the Proxy is not constructable.
    print('- target not callable');
    target = {};
    handler = {
        construct: function myConstruct(targ, argArray, newTarget) {
            print('construct trap');
            print(this === handler);
            print(targ === target);
            print(typeof argArray, argArray.length, argArray[0], argArray[1], argArray[2]);
            print(newTarget === proxy);
            return { foo: 'bar-trap' };
        }
    };
    proxy = new Proxy(target, handler);
    globalProxyRef = proxy;
    print(typeof proxy);
    print(Object.prototype.toString.call(proxy));
    try {
        print(new proxy(123).foo);
    } catch (e) {
        print(e.name);
    }

    // Similarly, if the target is callable but not constructable, the Proxy
    // is not constructable.  However, typeof will still be a 'function' because
    // the Proxy is callable.
    print('- target not constructable');
    target = Math.sin;
    proxy = new Proxy(target, handler);
    globalProxyRef = proxy;
    print(typeof proxy);
    print(Object.prototype.toString.call(proxy));
    try {
        print(new proxy(123).foo);
    } catch (e) {
        print(e.name);
    }

    // Proxy may be constructable but trap not callable.
    // Firefox ignores a null construct trap; ES2015 and later seem to
    // indicate only 'undefined' trap should be ignored.
    print('- proxy constructable but trap not callable');
    target = MyConstructor;
    proxy = new Proxy(target, { construct: null });
    globalProxyRef = proxy;
    print(typeof proxy);
    print(Object.prototype.toString.call(proxy));
    try {
        print(new proxy(123).foo);
    } catch (e) {
        print(e.name);
    }

    // Proxy may be constructable but trap only callable (not constructable),
    // this is OK as the trap is called as a normal function.  Trap is chosen
    // carefully to be a non-constructable built-in which returns an object
    // to satisfy the Proxy return value invariant.
    print('- proxy constructable but trap only callable');
    target = MyConstructor;
    proxy = new Proxy(target, { construct: Array.prototype.map });
    globalProxyRef = proxy;
    print(typeof proxy);
    print(Object.prototype.toString.call(proxy));
    try {
        print(new proxy(123).length);  // return value is empty array, so 0
    } catch (e) {
        print(e.name);
    }

    // Basic case.
    print('- basic case');
    target = MyConstructor;
    proxy = new Proxy(target, handler);
    globalProxyRef = proxy;
    print(typeof proxy);
    print(Object.prototype.toString.call(proxy));
    try {
        print(new proxy(123, 234).foo);
    } catch (e) {
        print(e.name);
    }

    // Trap is also called via Reflect.construct().
    print('- Reflect.construct');
    print(Reflect.construct(proxy, [ 123, 234 ], proxy).foo);

    // Passthrough case: Proxy without 'construct' trap.
    print('- passthrough');
    target = MyConstructor;
    handler = {};
    proxy = new Proxy(target, handler);
    globalProxyRef = proxy;
    print(typeof proxy);
    print(Object.prototype.toString.call(proxy));
    print(new proxy(123, 234).foo);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
