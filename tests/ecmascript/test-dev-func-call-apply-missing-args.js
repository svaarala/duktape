/*
 *  Some tests for .call() and .apply() with missing arguments.
 */

/*===
- Function.prototype.call()
undefined undefined undefined undefined
mythis undefined undefined undefined
mythis arg undefined undefined
- Function.prototype.apply()
undefined undefined undefined undefined
mythis undefined undefined undefined
mythis undefined undefined undefined
mythis undefined undefined undefined
mythis undefined undefined undefined
mythis arg undefined undefined
mythis undefined undefined undefined
mythis foo bar undefined
mythis FOO BAR undefined
TypeError
- Reflect.apply()
TypeError
TypeError
undefined undefined undefined undefined
mythis undefined undefined undefined
mythis undefined undefined undefined
mythis undefined undefined undefined
mythis undefined undefined undefined
mythis arg undefined undefined
mythis undefined undefined undefined
mythis foo bar undefined
mythis FOO BAR undefined
TypeError
done
===*/

function test() {
    function func(a,b,c) {
        'use strict';  // avoid 'this' coercion
        print(this, a, b, c);
    }

    var dummyFunc = function dummy(a,b) {};  // length: 2
    dummyFunc[0] = 'FOO';
    dummyFunc[1] = 'BAR';
    dummyFunc[2] = 'QUUX';

    // For .call() there are no required arguments.
    // This binding will be 'undefined'.
    print('- Function.prototype.call()');
    func.call();
    func.call('mythis');
    func.call('mythis', 'arg');

    // For .apply() there are similarly no required arguments.
    // argArray can be null/undefined and is treated like empty
    // array.  Array-like object is accepted (.length is respected);
    // other types cause a TypeError.  A function is an acceptable
    // input because it's an object and even has a .length!
    print('- Function.prototype.apply()');
    func.apply();
    func.apply('mythis');
    func.apply('mythis', void 0);
    func.apply('mythis', null);
    func.apply('mythis', []);
    func.apply('mythis', [ 'arg' ]);
    func.apply('mythis', {});  // no .length -> same as []
    func.apply('mythis', { length: 2, 0: 'foo', 1: 'bar', 2: 'quux-ignored' });
    func.apply('mythis', dummyFunc);
    try {
        func.apply('mythis', 123);
    } catch (e) {
        print(e.name);
    }

    // Reflect.apply() requires a callable first argument,
    // but is otherwise similar to Function.prototype.apply().
    print('- Reflect.apply()');
    try {
        Reflect.apply();
    } catch (e) {
        print(e.name);
    }
    try {
        Reflect.apply(123);
    } catch (e) {
        print(e.name);
    }
    Reflect.apply(func);
    Reflect.apply(func, 'mythis');
    Reflect.apply(func, 'mythis', void 0);
    Reflect.apply(func, 'mythis', null);
    Reflect.apply(func, 'mythis', []);
    Reflect.apply(func, 'mythis', [ 'arg' ]);
    Reflect.apply(func, 'mythis', {});
    Reflect.apply(func, 'mythis', { length: 2, 0: 'foo', 1: 'bar', 2: 'quux-ignored' });
    Reflect.apply(func, 'mythis', dummyFunc);
    try {
        Reflect.apply(func, 'mythis', 123);
    } catch (e) {
        print(e.name);
    }

    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
