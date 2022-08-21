/*===
TypeError
===*/

// Non-object argument causes a throw.
try {
    Reflect.preventExtensions(123);
} catch (e) {
    print(e.name);
}
