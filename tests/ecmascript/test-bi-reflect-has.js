/*
 *  Reflect.has()
 */

/*===
true
true
false
===*/

function test() {
    // Reflect.has() looks at own as well as inherited properties.
    var proto = { inherited: "inherited" };
    var obj = Object.create(proto, {
        own: { value: "own" }
    });
    print(Reflect.has(obj, 'own'));
    print(Reflect.has(obj, 'inherited'));
    print(Reflect.has(obj, 'nonexistent'));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
