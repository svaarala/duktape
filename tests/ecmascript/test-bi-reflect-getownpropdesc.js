/*
 *  Reflect.getOwnPropertyDescriptor()
 */

/*===
foobar true true true
value,writable,enumerable,configurable
undefined
===*/

function test() {
    var obj  = { foo: "foobar" };

    var desc = Reflect.getOwnPropertyDescriptor(obj, 'foo');
    print(desc.value, desc.writable, desc.enumerable, desc.configurable);
    print(Reflect.ownKeys(desc));

    // getOwnPropertyDescriptor() returns undefined for a nonexistent property:
    print(Reflect.getOwnPropertyDescriptor(obj, 'bar'));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
