/*===
basic
foo 3 true true true undefined undefined
bar 4 true true true undefined undefined
prop1 1 true true true undefined undefined
prop2 1 false true true undefined undefined
prop3 1 true false true undefined undefined
prop4 1 false false true undefined undefined
prop5 1 true true false undefined undefined
prop6 1 false true false undefined undefined
prop7 1 true false false undefined undefined
prop8 1 false false false undefined undefined
acc1 undefined undefined true true function function
acc2 undefined undefined false true function function
acc3 undefined undefined true false function function
acc4 undefined undefined false false function function
proto_foo undefined
proto_bar undefined
nonexistent undefined
===*/

print('basic');

function basicTest() {
    var proto = { proto_foo: 1, proto_bar: 2 };
    var obj;
    var names = [ 'foo', 'bar', 'prop1', 'prop2', 'prop3', 'prop4', 'prop5', 'prop6',
                  'prop7', 'prop8', 'acc1', 'acc2', 'acc3', 'acc4',
                  'proto_foo', 'proto_bar',  // prototype properties NOT given out
                  'nonexistent' ];
    var i;

    function test(o, n) {
        var pd = Object.getOwnPropertyDescriptor(o, n);
        if (pd === undefined) {
            print(n, 'undefined');
        } else {
            print(n, pd.value, pd.writable, pd.enumerable, pd.configurable, typeof pd.get, typeof pd.set);
        }
    }

    obj = Object.create(proto);
    obj.foo = 3;
    obj.bar = 4;

    Object.defineProperties(obj, {
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
        test(obj, names[i]);
    }
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
coercion
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
object foo undefined ok
object foo object 1
function foo undefined ok
object undefined object undefined-prop
object undefined object undefined-prop
object null object null-prop
object true object true-prop
object false object false-prop
object 123 object 123-prop
object foo object foo-prop
object 1,2 object array-prop
object [object Object] object object-prop
toString() called
===*/

print('coercion');

function coercionTest() {
    var obj = {
        foo: 'foo-prop',
        bar: 'bar-prop',
        undefined: 'undefined-prop',
        null: 'null-prop',
        true: 'true-prop',
        false: 'false-prop',
        "123": '123-prop',
        "1,2": 'array-prop',
        "[object Object]": 'object-prop'
    };

    function test(obj, prop, arg_count) {
        var t;

        try {
            if (arg_count === 0) {
                t = Object.getOwnPropertyDescriptor();
            } else if (arg_count === 1) {
                t = Object.getOwnPropertyDescriptor(obj);
            } else {
                t = Object.getOwnPropertyDescriptor(obj, prop);
            }
            print(typeof obj, prop, typeof t, (t !== undefined && t.value !== undefined ? t.value : 'ok'));
        } catch (e) {
            print(e.name);
        }
    }

    // first arg handling
    test(undefined, 'foo', 0);
    test(null, 'foo', 2);
    test(true, 'foo', 2);
    test(false, 'foo', 2);
    test(123, 'foo', 2);
    test('foo', 'foo', 2);
    test([1,2], 'foo', 2);
    test({ foo: 1, bar: 2 }, 'foo', 2);
    test(function(){}, 'foo', 2);

    // second arg handling
    test(obj, undefined, 1);
    test(obj, undefined, 2);
    test(obj, null, 2);
    test(obj, true, 2);
    test(obj, false, 2);
    test(obj, 123, 2);
    test(obj, 'foo', 2);
    test(obj, [1,2], 2);
    test(obj, { foo: 1, bar: 2 }, 2);

    // side effect test for second arg
    Object.getOwnPropertyDescriptor(obj, {
        toString: function() { print('toString() called'); return 'foo'; },
        valueOf: function() { print('valueOf() called'); return 'bar'; }
    });
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
