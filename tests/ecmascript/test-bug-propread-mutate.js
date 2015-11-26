/*
 *  For a property access the base value must be evaluated first to a safe
 *  temporary/constant which won't be changed if code in the key expression
 *  assigns to the variable holding the base value.
 */

/*===
obj1
obj1
obj1
===*/

function test() {
    var obj1 = { name: 'obj1' };
    var obj2 = { name: 'obj2' };

    // For these it'd be nice to avoid an explicit bytecode copy, because
    // the key won't mutate any state.
    print(obj1.name);
    print(obj1['name']);

    // Here a copy must be made.
    print(obj1[(obj1 = obj2), 'name']);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
