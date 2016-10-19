/*
 *  Reflect.ownKeys()
 */

/*===
prop1,prop2
prop2
prop1,prop2
===*/

function test() {
    'use strict';

    var obj = { prop1: "foo", prop2: "bar" };

    print(Reflect.ownKeys(obj));

    // Reflect.ownKeys() includes non-enumerable properties, unlike
    // Object.keys().
    Reflect.defineProperty(obj, 'prop1', { enumerable: false });
    print(Object.keys(obj));
    print(Reflect.ownKeys(obj));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
