/*
 *  Tests for Object isSealed(), isFrozen(), isExtensible(), seal(),
 *  and freeze().
 */

/*===
sealed: false, frozen: false, extensible: true
sealed: true, frozen: false, extensible: false
sealed: false, frozen: false, extensible: true
sealed: true, frozen: true, extensible: false
sealed: false, frozen: false, extensible: true
sealed: true, frozen: false, extensible: false
sealed: false, frozen: false, extensible: true
sealed: true, frozen: true, extensible: false
===*/

function basicTest() {
    var obj;

    function p(o) {
        print('sealed: ' + Object.isSealed(o) +
              ', frozen: ' + Object.isFrozen(o) +
              ', extensible: ' + Object.isExtensible(o));
    }

    obj = { foo: 1, bar: 2 };
    p(obj);
    Object.seal(obj);
    p(obj);

    obj = { foo: 1, bar: 2 };
    p(obj);
    Object.freeze(obj);
    p(obj);

    // create objects which are already sealed/frozen(), without a
    // seal() or freeze() call.

    obj = Object.create(Object.prototype, {
        prop1: { value: 123, writable: true, enumerable: true, configurable: false },
        prop2: { get: function(){}, set: function(){}, enumerable: true, configurable: false }
    });
    p(obj);  // extensible -> not sealed or frozen
    Object.preventExtensions(obj);  // now sealed, but not frozen (writable property exists)
    p(obj);

    // sealed and frozen
    obj = Object.create(Object.prototype, {
        prop1: { value: 123, writable: false, enumerable: true, configurable: false },
        prop2: { get: function(){}, set: function(){}, enumerable: true, configurable: false }
    });
    p(obj);  // extensible -> not sealed or frozen
    Object.preventExtensions(obj);  // now sealed and frozen: no writable properties, accessors are OK
    p(obj);
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
object, #names: 12, extensible: true
index: 0, name: prop1, value: 1, writable: true, enumerable: true, configurable: true, get: undefined, set: undefined
index: 1, name: prop2, value: 2, writable: false, enumerable: true, configurable: true, get: undefined, set: undefined
index: 2, name: prop3, value: 3, writable: true, enumerable: false, configurable: true, get: undefined, set: undefined
index: 3, name: prop4, value: 4, writable: false, enumerable: false, configurable: true, get: undefined, set: undefined
index: 4, name: prop5, value: 5, writable: true, enumerable: true, configurable: false, get: undefined, set: undefined
index: 5, name: prop6, value: 6, writable: false, enumerable: true, configurable: false, get: undefined, set: undefined
index: 6, name: prop7, value: 7, writable: true, enumerable: false, configurable: false, get: undefined, set: undefined
index: 7, name: prop8, value: 8, writable: false, enumerable: false, configurable: false, get: undefined, set: undefined
index: 8, name: acc1, value: undefined, writable: undefined, enumerable: true, configurable: true, get: function, set: function
index: 9, name: acc2, value: undefined, writable: undefined, enumerable: false, configurable: true, get: function, set: function
index: 10, name: acc3, value: undefined, writable: undefined, enumerable: true, configurable: false, get: function, set: function
index: 11, name: acc4, value: undefined, writable: undefined, enumerable: false, configurable: false, get: function, set: function
object, #names: 12, extensible: false
index: 0, name: prop1, value: 1, writable: true, enumerable: true, configurable: false, get: undefined, set: undefined
index: 1, name: prop2, value: 2, writable: false, enumerable: true, configurable: false, get: undefined, set: undefined
index: 2, name: prop3, value: 3, writable: true, enumerable: false, configurable: false, get: undefined, set: undefined
index: 3, name: prop4, value: 4, writable: false, enumerable: false, configurable: false, get: undefined, set: undefined
index: 4, name: prop5, value: 5, writable: true, enumerable: true, configurable: false, get: undefined, set: undefined
index: 5, name: prop6, value: 6, writable: false, enumerable: true, configurable: false, get: undefined, set: undefined
index: 6, name: prop7, value: 7, writable: true, enumerable: false, configurable: false, get: undefined, set: undefined
index: 7, name: prop8, value: 8, writable: false, enumerable: false, configurable: false, get: undefined, set: undefined
index: 8, name: acc1, value: undefined, writable: undefined, enumerable: true, configurable: false, get: function, set: function
index: 9, name: acc2, value: undefined, writable: undefined, enumerable: false, configurable: false, get: function, set: function
index: 10, name: acc3, value: undefined, writable: undefined, enumerable: true, configurable: false, get: function, set: function
index: 11, name: acc4, value: undefined, writable: undefined, enumerable: false, configurable: false, get: function, set: function
object, #names: 12, extensible: true
index: 0, name: prop1, value: 1, writable: true, enumerable: true, configurable: true, get: undefined, set: undefined
index: 1, name: prop2, value: 2, writable: false, enumerable: true, configurable: true, get: undefined, set: undefined
index: 2, name: prop3, value: 3, writable: true, enumerable: false, configurable: true, get: undefined, set: undefined
index: 3, name: prop4, value: 4, writable: false, enumerable: false, configurable: true, get: undefined, set: undefined
index: 4, name: prop5, value: 5, writable: true, enumerable: true, configurable: false, get: undefined, set: undefined
index: 5, name: prop6, value: 6, writable: false, enumerable: true, configurable: false, get: undefined, set: undefined
index: 6, name: prop7, value: 7, writable: true, enumerable: false, configurable: false, get: undefined, set: undefined
index: 7, name: prop8, value: 8, writable: false, enumerable: false, configurable: false, get: undefined, set: undefined
index: 8, name: acc1, value: undefined, writable: undefined, enumerable: true, configurable: true, get: function, set: function
index: 9, name: acc2, value: undefined, writable: undefined, enumerable: false, configurable: true, get: function, set: function
index: 10, name: acc3, value: undefined, writable: undefined, enumerable: true, configurable: false, get: function, set: function
index: 11, name: acc4, value: undefined, writable: undefined, enumerable: false, configurable: false, get: function, set: function
object, #names: 12, extensible: false
index: 0, name: prop1, value: 1, writable: false, enumerable: true, configurable: false, get: undefined, set: undefined
index: 1, name: prop2, value: 2, writable: false, enumerable: true, configurable: false, get: undefined, set: undefined
index: 2, name: prop3, value: 3, writable: false, enumerable: false, configurable: false, get: undefined, set: undefined
index: 3, name: prop4, value: 4, writable: false, enumerable: false, configurable: false, get: undefined, set: undefined
index: 4, name: prop5, value: 5, writable: false, enumerable: true, configurable: false, get: undefined, set: undefined
index: 5, name: prop6, value: 6, writable: false, enumerable: true, configurable: false, get: undefined, set: undefined
index: 6, name: prop7, value: 7, writable: false, enumerable: false, configurable: false, get: undefined, set: undefined
index: 7, name: prop8, value: 8, writable: false, enumerable: false, configurable: false, get: undefined, set: undefined
index: 8, name: acc1, value: undefined, writable: undefined, enumerable: true, configurable: false, get: function, set: function
index: 9, name: acc2, value: undefined, writable: undefined, enumerable: false, configurable: false, get: function, set: function
index: 10, name: acc3, value: undefined, writable: undefined, enumerable: true, configurable: false, get: function, set: function
index: 11, name: acc4, value: undefined, writable: undefined, enumerable: false, configurable: false, get: function, set: function
===*/

function propertyAttributeTest() {
    var obj;
    var prop_names = [ 'prop1', 'prop2', 'prop3', 'prop4',
                       'prop5', 'prop6', 'prop7', 'prop8',
                       'acc1', 'acc2', 'acc3', 'acc4' ];

    function mkGetter() {
        return function getter() {
            print('getter called');
            throw new Error('unexpected getter call');
        };
    }
    function mkSetter() {
        return function getter() {
            print('setter called');
            throw new Error('unexpected setter call');
        };
    }
    function mkObj() {
        var obj = {};
        Object.defineProperties(obj, {
            prop1: { value: 1, writable: true, enumerable: true, configurable: true },
            prop2: { value: 2, writable: false, enumerable: true, configurable: true },
            prop3: { value: 3, writable: true, enumerable: false, configurable: true },
            prop4: { value: 4, writable: false, enumerable: false, configurable: true },
            prop5: { value: 5, writable: true, enumerable: true, configurable: false },
            prop6: { value: 6, writable: false, enumerable: true, configurable: false },
            prop7: { value: 7, writable: true, enumerable: false, configurable: false },
            prop8: { value: 8, writable: false, enumerable: false, configurable: false },
            acc1: { get: mkGetter(), set: mkSetter(), enumerable: true, configurable: true },
            acc2: { get: mkGetter(), set: mkSetter(), enumerable: false, configurable: true },
            acc3: { get: mkGetter(), set: mkSetter(), enumerable: true, configurable: false },
            acc4: { get: mkGetter(), set: mkSetter(), enumerable: false, configurable: false },
        });

/*
    For some reason V8 (at least 3.7.12.22) reorders these properties regardless
    of whether one uses defineProperty() or defineProperties().  So, we read back
    properties with explicitly ordered names below instead of relying on
    Object.getOwnPropertyNames().

        Object.defineProperty(obj, 'prop1', { value: 1, writable: true, enumerable: true, configurable: true });
        Object.defineProperty(obj, 'prop2', { value: 2, writable: false, enumerable: true, configurable: true });
        Object.defineProperty(obj, 'prop3', { value: 3, writable: true, enumerable: false, configurable: true });
        Object.defineProperty(obj, 'prop4', { value: 4, writable: false, enumerable: false, configurable: true });
        Object.defineProperty(obj, 'prop5', { value: 5, writable: true, enumerable: true, configurable: false });
        Object.defineProperty(obj, 'prop6', { value: 6, writable: false, enumerable: true, configurable: false });
        Object.defineProperty(obj, 'prop7', { value: 7, writable: true, enumerable: false, configurable: false });
        Object.defineProperty(obj, 'prop8', { value: 8, writable: false, enumerable: false, configurable: false });
        Object.defineProperty(obj, 'acc1', { get: mkGetter(), set: mkSetter(), enumerable: true, configurable: true });
        Object.defineProperty(obj, 'acc2', { get: mkGetter(), set: mkSetter(), enumerable: false, configurable: true });
        Object.defineProperty(obj, 'acc3', { get: mkGetter(), set: mkSetter(), enumerable: true, configurable: false });
        Object.defineProperty(obj, 'acc4', { get: mkGetter(), set: mkSetter(), enumerable: false, configurable: false });
*/

        return obj;
    }
    function printObj(o) {
        var i, pd;

        names = Object.getOwnPropertyNames(o);
        print('object, #names: ' + names.length + ', extensible: ' + Object.isExtensible(o));

        /* Use explicit names because V8 loses property order for some reason */

        for (i = 0; i < prop_names.length; i++) {
            pd = Object.getOwnPropertyDescriptor(o, prop_names[i]);
            print('index: ' + i +
                  ', name: ' + prop_names[i] +
                  ', value: ' + pd.value +
                  ', writable: ' + pd.writable +
                  ', enumerable: ' + pd.enumerable +
                  ', configurable: ' + pd.configurable +
                  ', get: ' + typeof pd.get +
                  ', set: ' + typeof pd.set);
        }
    }

    obj = mkObj();
    printObj(obj);
    Object.seal(obj);
    printObj(obj);

    obj = mkObj();
    printObj(obj);
    Object.freeze(obj);
    printObj(obj);
}

try {
    propertyAttributeTest();
} catch (e) {
    print(e);
}

/*===
0 0
TypeError
0 1
TypeError
0 2
TypeError
0 3
TypeError
0 4
TypeError
0 5
TypeError
0 6
ok
0 7
ok
1 0
TypeError
1 1
TypeError
1 2
TypeError
1 3
TypeError
1 4
TypeError
1 5
TypeError
1 6
ok
1 7
ok
2 0
TypeError
2 1
TypeError
2 2
TypeError
2 3
TypeError
2 4
TypeError
2 5
TypeError
2 6
ok
2 7
ok
3 0
TypeError
3 1
TypeError
3 2
TypeError
3 3
TypeError
3 4
TypeError
3 5
TypeError
3 6
ok
3 7
ok
4 0
TypeError
4 1
TypeError
4 2
TypeError
4 3
TypeError
4 4
TypeError
4 5
TypeError
4 6
ok
4 7
ok
===*/

function coercionTest() {
    var funcs = [
        Object.isSealed, Object.isFrozen, Object.isExtensible,
        Object.seal, Object.freeze
    ];
    var values = [
        undefined, null, true, false, 123, 'foo',
        [1,2], { foo: 1, bar: 2 }
    ];
    var i, j;

    function test(func, obj) {
        try {
            func(obj);
            print('ok');
        } catch (e) {
            print(e.name);
        }
    }

    for (i = 0; i < funcs.length; i++) {
        for (j = 0; j < values.length; j++) {
            print(i, j);
            test(funcs[i], values[j]);
        }
    }
}

try {
    coercionTest();
} catch (e) {
    print(e);
}

/* XXX: test that ancestors have no effect and or not affected */
