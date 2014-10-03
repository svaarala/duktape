/*===
basic
object string boolean true number
object string boolean true number
object string boolean false string
object string boolean false undefined
===*/

print('basic');

function basicTest() {
    var proto = { proto: 'prototype' };
    var obj;

    function test(o, n) {
        var t = o.hasOwnProperty(n);
        print(typeof o, typeof n, typeof t, t, typeof o[n]);
    }

    obj = Object.create(proto);
    obj.foo = 1;
    obj.bar = 2;

    // own property
    test(obj, 'foo');
    test(obj, 'bar');

    // ancestor property (not found)
    test(obj, 'proto');

    // non-existent property
    test(obj, 'non-existent');
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
            t = Object.prototype.hasOwnProperty.call(this_value, prop_name);
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
    // only happens after ToString() is called for property name (V8 does not
    // seem to do this)

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
