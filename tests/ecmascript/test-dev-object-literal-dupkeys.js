/*
 *  ES5 prohibits duplicate keys in object literals.  ES2015 allows them:
 *  last occurrence wins, and side effects must occur.
 */

/*===
aiee 1
aiee 2
345
aiee 1
aiee 2
foovar
foo getter
undefined
function undefined undefined true
1
undefined undefined true true
foo getter 2
undefined
foo setter
function function undefined true
foo getter 2
undefined
function undefined undefined true
===*/

function test() {
    'use strict';  // duplicates allowed even in strict mode

    var foo = 'foovar';
    var obj;
    var pd;

    obj = {
        foo: 123,
        foo: (print('aiee 1'), 234),
        foo: (print('aiee 2'), 345)
    };
    print(obj.foo);

    obj = {
        foo: 123,
        foo: (print('aiee 1'), 234),
        foo: (print('aiee 2'), 345),
        foo
    };
    print(obj.foo);

    obj = {
        foo: 1,
        get foo() { print('foo getter') }
    };
    print(obj.foo);
    pd = Object.getOwnPropertyDescriptor(obj, 'foo');
    print(typeof pd.get, typeof pd.set, pd.writable, pd.configurable);

    obj = {
        get foo() { print('foo getter') },
        foo: 1
    };
    print(obj.foo);
    pd = Object.getOwnPropertyDescriptor(obj, 'foo');
    print(typeof pd.get, typeof pd.set, pd.writable, pd.configurable);

    obj = {
        get foo() { print('foo getter') },
        set foo(v) { print('foo setter') },
        // this updates the getter - but the setter is kept
        get foo() { print('foo getter 2') },
    };
    print(obj.foo);
    obj.foo = 123;
    pd = Object.getOwnPropertyDescriptor(obj, 'foo');
    print(typeof pd.get, typeof pd.set, pd.writable, pd.configurable);

    obj = {
        get foo() { print('foo getter') },
        set foo(v) { print('foo setter') },
        foo,  // this resets both setter and getter
        get foo() { print('foo getter 2') },
    };
    print(obj.foo);
    pd = Object.getOwnPropertyDescriptor(obj, 'foo');
    print(typeof pd.get, typeof pd.set, pd.writable, pd.configurable);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
