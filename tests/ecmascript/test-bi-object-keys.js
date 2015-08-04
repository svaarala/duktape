/*===
basic
foo
bar
proto
0 foo
1 bar
===*/

print('basic');

function basicTest() {
    var proto = { proto: 'prototype' };
    var obj;
    var keys;
    var i;

    obj = Object.create(proto);
    obj.foo = 1;
    obj.bar = 2;

    Object.defineProperties(proto, {
        proto_nonenum: { value: 1, writable: true, enumerable: false, configurable: true }
    });

    Object.defineProperties(obj, {
        nonenum: { value: 1, writable: true, enumerable: false, configurable: true }
    });


    // Here we make an assumption on enumeration order.
    // Enumeration includes prototype 'proto' property too.

    for (i in obj) {
        print(i);
    }

    // Object.keys() only includes own enumerable properties.

    keys = Object.keys(obj);
    for (i = 0; i < keys.length; i++) {
        print(i, keys[i]);
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
TypeError
object 2
object 2
===*/

print('coercion');

function coercionTest() {
    function test(o, is_noargs) {
        var t;

        try {
            if (is_noargs) {
                t = Object.keys();
            } else {
                t = Object.keys(o);
            }
            print(typeof t, t.length);
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
    test([1,2]);
    test({ foo: 1, bar: 2 });
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
