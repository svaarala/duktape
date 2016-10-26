/*
 *  Reflect.deleteProperty()
 */

/*===
true
false
true
false
true
true
===*/

function test() {
    'use strict';

    var eaty = { pig: "maggie", cow: "Kittycow", ape: "Machel" };

    print(Reflect.deleteProperty(eaty, 'pig'));
    print('pig' in eaty);  // false (The cow did it.)

    // deleteProperty() for nonexistent key returns true, like the delete
    // operator:
    print(Reflect.deleteProperty(eaty, 'pig'));  // true

    // Trying to delete a non-configurable property will fail.  It will NOT
    // throw however, even when the caller (that's us!) is strict:
    Reflect.defineProperty(eaty, 'ape', { configurable: false });
    print(Reflect.deleteProperty(eaty, 'ape'));
    print('ape' in eaty);  // Still here!

    print(Reflect.deleteProperty(eaty, 'cow'));  // The ape did it.
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
