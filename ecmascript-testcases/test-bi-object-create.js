/*===
firstarg
TypeError
TypeError
ok
TypeError
TypeError
TypeError
TypeError
ok
ok
ok
prototype: null
prototype: [object Object] true prototype
===*/

/* First argument must be null or Object; TypeError otherwise */

print('firstarg');

function firstArgTest() {
    var obj;
    var proto;

    function test(arg, is_noarg) {
        var t;

        try {
            if (is_noarg) {
                t = Object.create();
            } else {
                t = Object.create(arg);
            }
            print('ok');
        } catch (e) {
            print(e.name);
        }
    }

    test(undefined, true);
    test(undefined);
    test(null);
    test(true);
    test(false);
    test(123);
    test('foo');
    test([1,2,3]);
    test({ foo: 1, bar: 2});
    test(function(){});

    // when set as null or a specific object, must become the internal
    // prototype

    obj = Object.create(null);
    print('prototype: ' + Object.getPrototypeOf(obj));

    proto = { proto: 'prototype' };
    obj = Object.create(proto);
    print('prototype: ' + Object.getPrototypeOf(obj), Object.getPrototypeOf(obj) === proto, obj.proto);
}

try {
    firstArgTest();
} catch (e) {
    print(e);
}

/*===
property attributes
0 prop1 1 true true true undefined undefined
1 prop2 1 false true true undefined undefined
2 prop3 1 true false true undefined undefined
3 prop4 1 false false true undefined undefined
4 prop5 1 true true false undefined undefined
5 prop6 1 false true false undefined undefined
6 prop7 1 true false false undefined undefined
7 prop8 1 false false false undefined undefined
8 acc1 undefined undefined true true function function
9 acc2 undefined undefined false true function function
10 acc3 undefined undefined true false function function
11 acc4 undefined undefined false false function function
===*/

print('property attributes');

function propertyAttributeTest() {
    var proto = {};
    var obj;
    var names = [ 'prop1', 'prop2', 'prop3', 'prop4',
                  'prop5', 'prop6', 'prop7', 'prop8',
                  'acc1', 'acc2', 'acc3', 'acc4' ];
    var pd;

    obj = Object.create(proto, {
        prop1: { value: 1, writable: true, enumerable: true, configurable: true },
        prop2: { value: 1, writable: false, enumerable: true, configurable: true },
        prop3: { value: 1, writable: true, enumerable: false, configurable: true },
        prop4: { value: 1, writable: false, enumerable: false, configurable: true },
        prop5: { value: 1, writable: true, enumerable: true, configurable: false },
        prop6: { value: 1, writable: false, enumerable: true, configurable: false },
        prop7: { value: 1, writable: true, enumerable: false, configurable: false },
        prop8: { value: 1, writable: false, enumerable: false, configurable: false },
        acc1: { get: function(){}, set: function(){}, enumerable: true, configurable: true },
        acc2: { get: function(){}, set: function(){}, enumerable: false, configurable: true },
        acc3: { get: function(){}, set: function(){}, enumerable: true, configurable: false },
        acc4: { get: function(){}, set: function(){}, enumerable: false, configurable: false }
    });

    for (i = 0; i < names.length; i++) {
        pd = Object.getOwnPropertyDescriptor(obj, names[i]);
        print(i, names[i], pd.value, pd.writable, pd.enumerable, pd.configurable,
              typeof pd.get, typeof pd.set);
    }
}

try {
    propertyAttributeTest();
} catch (e) {
    print(e);
}

/*===
object replaced
object prototype foo bar quux
===*/

/* The object should be created with the standard built-in Object constructor,
 * even if Object had been replaced.  Also test that the standard
 * Object.defineProperties() behavior works even if Object.defineProperties is
 * bogus.
 */

print('object replaced');

function objectReplacedTest() {
    var Object_old;
    var Object_defineProperties_old;

    Object_old = Object;
    Object_defineProperties_old = Object.defineProperties;

    Object.defineProperties = function() {
        print('replacement defineProperties() in original Object called');
        throw new Error('never here');
    }

    Object = function() {
        print('replacement Object constructor called');
        throw new Error('never here');
    };

    Object.defineProperties = function () {
        print('defineProperties() in replacement Object called');
        throw new Error('never here');
    }

    obj = Object_old.create({ proto: 'prototype' }, {
        foo: { value: 'foo', writable: true, enumerable: true, configurable: true },
        bar: { value: 'bar', writable: true, enumerable: true, configurable: true },
        quux: { value: 'quux', writable: true, enumerable: false, configurable: true },
    });

    Object = Object_old;
    Object.defineProperties = Object_defineProperties_old;

    print(typeof obj, obj.proto, obj.foo, obj.bar, obj.quux);
}

try {
    objectReplacedTest();
} catch (e) {
    print(e);
}
