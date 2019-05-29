/*
 *  Proxy (ES2015) 'apply'.
 */

/*===
- target not callable
object
TypeError
- target callable, trap non-callable
function
TypeError
- target callable, proxy callable
function
apply trap
true
true
undefined
object 2 123 234 undefined
true
VALUE
- .call(), .apply(), Reflect.apply()
apply trap
true
true
myThisCall
object 4 123 234 345
true
VALUE
apply trap
true
true
myThisApply
object 2 123 234 undefined
true
VALUE
apply trap
true
true
myThisReflectApply
object 2 123 234 undefined
true
VALUE
- bound function as trap
function
bound apply trap
true
123 234
VALUE
- passthrough
function
myFunction called
object [object global]
foo bar quux undefined undefined
123
===*/

function myFunction(a,b,c,d,e) {
    print('myFunction called');
    print(typeof this, this);
    print(a, b, c, d, e);
    return 123;
}

function test() {
    var target;
    var handler;
    var proxy;

    // If the (initial) target is not callable, the Proxy is not callable
    // even if it has a callable 'apply' trap.
    print('- target not callable');
    target = {};
    handler = {
        apply: function myApply(targ, thisArg, argArray) {
            print('apply trap');
            print(this === handler);
            print(targ === target);
            print(thisArg);
            print(typeof argArray, argArray.length, argArray[0], argArray[1], argArray[2]);
            return 'VALUE';
        }
    };
    proxy = new Proxy(target, handler);
    print(typeof proxy);
    try {
        print(proxy(123));
    } catch (e) {
        print(e.name);
    }

    // The Proxy may be callable but a trap non-callable; if so, TypeError.
    // Firefox ignores both 'null' and 'undefined', specification only seems
    // to ignore missing/undefined.
    print('- target callable, trap non-callable');
    target = myFunction;
    handler = {
        apply: null
    };
    proxy = new Proxy(target, handler);
    print(typeof proxy);
    try {
        print(proxy(123, 234));
    } catch (e) {
        print(e.name);
    }

    // If the (initial) target is callable, the Proxy is callable which also
    // makes typeof return 'function'.
    print('- target callable, proxy callable');
    target = myFunction;
    handler = {
        apply: function myApply(targ, thisArg, argArray) {
            print('apply trap');
            print(this === handler);
            print(targ === target);
            print(thisArg);
            print(typeof argArray, argArray.length, argArray[0], argArray[1], argArray[2]);
            print(Object.getPrototypeOf(argArray) === Array.prototype);
            return 'VALUE';
        }
    };
    proxy = new Proxy(target, handler);
    print(typeof proxy);
    try {
        print(proxy(123, 234));
    } catch (e) {
        print(e.name);
    }

    // Trap is also called via .call(), .apply(), and Reflect.apply();
    print('- .call(), .apply(), Reflect.apply()');
    print(proxy.call('myThisCall', 123, 234, 345, 456));
    print(proxy.apply('myThisApply', [ 123, 234 ]));
    print(Reflect.apply(proxy, 'myThisReflectApply', [ 123, 234 ]));

    // Bound function as trap.
    print('- bound function as trap');
    target = myFunction;
    function myApply(targ, thisArg, argArray) {
        // Two args are bound, so actual 'targ' argument ends at argArray.
        print('bound apply trap');
        print(argArray === target);
        print(targ, thisArg);  // 123, 234
        return 'VALUE';
    }
    handler = {
        apply: myApply.bind('boundThis', 123, 234)
    };
    proxy = new Proxy(target, handler);
    print(typeof proxy);
    try {
        print(proxy(123, 234));
    } catch (e) {
        print(e.name);
    }

    // Passthrough case: Proxy without 'apply' trap.
    print('- passthrough');
    handler = {};
    target = myFunction;
    proxy = new Proxy(target, handler);
    print(typeof proxy);
    print(proxy('foo', 'bar', 'quux'));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
