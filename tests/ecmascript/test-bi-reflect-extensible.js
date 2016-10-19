/*
 *  Reflect.preventExtensions()
 *  Reflect.isExtensible()
 */

/*===
true
true
false
true
TypeError
false
MOOOOOoooooo... *munch*
false
undefined
===*/

function test() {
    'use strict';

    var eaty = { pig: "maggie", cow: "Kittycow" };

    print(Reflect.isExtensible(eaty));  // from object literal, so true
    print(Reflect.preventExtensions(eaty));
    print(Reflect.isExtensible(eaty));  // now false
    print(Reflect.preventExtensions(eaty));  // nop, but true anyway

    // Any assignment which creates a new property will now fail:
    try {
        eaty.ape = "Machel";  // TypeError
        print("never here");
    } catch (e) {
        print(e.name);
    }

    // Swapping out the prototype won't work either:
    print(Reflect.setPrototypeOf(eaty, { ape: "Machel" }));  // false

    // However, the object has not been sealed or frozen, so changing existing
    // properties is OK:
    eaty.cow = "MOOOOOoooooo... *munch*";
    print(eaty.cow);

    // ...as is deleting them, strangely enough:
    delete eaty.pig;
    print('pig' in eaty);  // false (The cow did it.)
    print(eaty.pig);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
