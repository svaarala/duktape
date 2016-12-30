/*
 *  Function instances
 */

/* XXX: toString(), call(), apply() */

/*===
instance
true true
true false false
true true
true false true
true 3
false false true
true test
false false true
""
true false
""
true false
"forcedName2"
true true
===*/

function functionInstanceTest() {
    var f = function test(a, b, c) {};
    var pd;

    print('prototype' in f, typeof f.prototype === 'object');
    pd = Object.getOwnPropertyDescriptor(f, 'prototype');
    print(pd.writable, pd.enumerable, pd.configurable);

    print('constructor' in f.prototype, f.prototype.constructor === f);
    pd = Object.getOwnPropertyDescriptor(f.prototype, 'constructor');
    print(pd.writable, pd.enumerable, pd.configurable);

    print('length' in f, f.length);
    pd = Object.getOwnPropertyDescriptor(f, 'length');
    print(pd.writable, pd.enumerable, pd.configurable);

    // Name added in ES2015.
    print('name' in f, f.name);
    pd = Object.getOwnPropertyDescriptor(f, 'name');
    print(pd.writable, pd.enumerable, pd.configurable);

    // Anonymous functions don't have an 'own' .name property but inherit
    // an empty string name from Function.prototype.  The property .name in
    // Function.prototype is not writable, but it is configurable, so that
    // Object.defineProperty() can be used to add a name later on.

    var anon = (1, 2, 3, function () {});
    print(JSON.stringify(anon.name));  // inherited
    print('name' in anon, anon.hasOwnProperty('name'));
    anon.name = 'forcedName';  // ineffective, not configurable
    print(JSON.stringify(anon.name));
    print('name' in anon, anon.hasOwnProperty('name'));
    Object.defineProperty(anon, 'name', { value: 'forcedName2' });
    print(JSON.stringify(anon.name));
    print('name' in anon, anon.hasOwnProperty('name'));
}

try {
    print('instance');
    functionInstanceTest();
} catch (e) {
    print(e.stack || e);
}
