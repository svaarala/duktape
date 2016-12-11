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

    // Name added in ES6.
    print('name' in f, f.name);
    pd = Object.getOwnPropertyDescriptor(f, 'name');
    print(pd.writable, pd.enumerable, pd.configurable);
}

try {
    print('instance');
    functionInstanceTest();
} catch (e) {
    print(e.stack || e);
}
