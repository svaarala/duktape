/*
 *  Test for non-standard property key argument for setter/getter
 *  calls.
 */

/*---
{
    "custom": true,
    "specialoptions": "requires non-standard behavior to be turned on"
}
---*/

/*===
myGetter true string foo
obj.foo: FOO
myGetter true string bar
obj["bar"]: BAR
obj.foo = 123
mySetter true number 123 string foo
obj["bar"] = 321
mySetter true number 321 string bar
myGetter true string 1
obj[1] 1
myGetter true string 1
obj["1"] 1
===*/

function nonStandardAccessorKeyArgumentTest() {
    var obj;

    function myGetter(key) {
        // 'this' binding: target object (standard)
        // key: property name (non-standard)
        print('myGetter', this === obj, typeof key, key);
        return (typeof key === 'string' ? key.toUpperCase() : key);
    }
    function mySetter(val, key) {
        // 'this' binding: target object (standard)
        // val: property value (standard)
        // key: property name (non-standard)
        print('mySetter', this === obj, typeof val, val, typeof key, key);
    }

    obj = {};
    Object.defineProperties(obj, {
        'foo': { enumerable: true, configurable: true, get: myGetter, set: mySetter },
        'bar': { enumerable: true, configurable: true, get: myGetter, set: mySetter },
        '1': { enumerable: true, configurable: true, get: myGetter, set: mySetter }
    });

    // Normal property reads

    print('obj.foo:', obj.foo);
    print('obj["bar"]:', obj['bar']);

    // Normal property writes

    print('obj.foo = 123');
    obj.foo = 123;
    print('obj["bar"] = 321');
    obj['bar'] = 321;

    // Numeric indices: these get coerced to a string in the current
    // implementation before reaching the accessor.  This is not ideal
    // but accessors are not a good way to virtualize array indices
    // anyway (Proxy object is much more appropriate), so this is not
    // a big limitation.

    print('obj[1]', obj[1]);
    print('obj["1"]', obj['1']);
}

try {
    nonStandardAccessorKeyArgumentTest();
} catch (e) {
    print(e);
}
