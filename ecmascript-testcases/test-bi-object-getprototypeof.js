/*===
basic
object object
object object true prototype
===*/

print('basic');

function basicTest() {
    var obj;
    var proto;
    var t;

    proto = null;
    obj = Object.create(proto);
    t = Object.getPrototypeOf(obj);
    print(typeof obj, typeof t);

    proto = { proto: 'prototype' };
    obj = Object.create(proto);
    t = Object.getPrototypeOf(obj);
    print(typeof obj, typeof t, t === proto, obj.proto);
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
firstarg
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
ok
ok
ok
===*/

print('firstarg');

function firstArgTest() {
    function test(arg, is_noarg) {
        var t;

        try {
            if (is_noarg) {
                t = Object.getPrototypeOf();
            } else {
                t = Object.getPrototypeOf(arg);
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
    test({ foo: 1, bar: 2 });
    test(function(){});
}

try {
    firstArgTest();
} catch (e) {
    print(e);
}
