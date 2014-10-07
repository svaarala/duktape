/*===
basic
0 foo true
1 bar true
2 prop1 true
3 prop2 true
4 prop3 false
5 prop4 false
6 prop5 true
7 prop6 true
8 prop7 false
9 prop8 false
10 acc1 true
11 acc2 false
12 acc3 true
13 acc4 false
14 proto_foo false
15 proto_bar false
16 nonexistent false
===*/

print('basic');

function basicTest() {
    var proto = { proto_foo: 1, proto_bar: 2 };
    var obj;
    var names = [ 'foo', 'bar', 'prop1', 'prop2', 'prop3', 'prop4', 'prop5', 'prop6',
                  'prop7', 'prop8', 'acc1', 'acc2', 'acc3', 'acc4',
                  'proto_foo', 'proto_bar',  // prototype properties NOT given out
                  'nonexistent' ];

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
        print(i, names[i], obj.propertyIsEnumerable(names[i]));
    }
}

try {
    basicTest();
} catch (e) {
    print(e);
}

/*===
coercion
toString() name
TypeError
toString() name
TypeError
toString() name
boolean boolean false
toString() name
boolean boolean false
toString() name
number boolean false
toString() name
object boolean false
toString() name
object boolean false
toString() name
object boolean false
===*/

print('coercion');

function coercionTest() {
    var objThis, objProp;

    function test(this_value, prop_name) {
        try {
            t = Object.prototype.propertyIsEnumerable.call(this_value, prop_name);
            print(typeof this_value, typeof t, t);
        } catch (e) {
            print(e.name);
        }
    }

    objThis = {
        toString: function () { print('toString() this'); return 'tostring-this'; },
        valueOf: function () { print('valueOf() this'); return 'valueof-this'; }
    };

    objProp = {
        toString: function () { print('toString() name'); return 'tostring-name'; },
        valueOf: function () { print('valueOf() name'); return 'valueof-name'; }
    };

    // ToObject(this) will throw a TypeError for undefined and null, but this
    // only happens after ToString() is called for property name

    test(undefined, objProp);
    test(null, objProp);
    test(true, objProp);
    test(false, objProp);
    test(123, objProp);
    test([1,2], objProp);
    test({ foo: 1, bar: 2 }, objProp);

    // First argument is already an object so it won't get coerced; second
    // argument gets coerced to a string

    test(objThis, objProp);
}

try {
    coercionTest();
} catch (e) {
    print(e);
}
