/*
 *  ES2015 method definition in object literal, with computed name.
 */

/*===
foo() sees foo as: undefined
5
function
string foo false false true
number 2 false false true
undefined
TypeError
5
==*/

function test() {
    var bar = 234;
    var obj;

    function getO() { return 'o'; }

    try {
        obj = eval('({ ["fo" + getO()](a,b) { print("foo() sees foo as:", typeof foo); return a+b; } })');

        print(obj.foo(2,3));
        print(typeof obj.foo);
        pd = Object.getOwnPropertyDescriptor(obj.foo, 'name');
        print(typeof pd.value, pd.value, pd.writable, pd.enumerable, pd.configurable);
        pd = Object.getOwnPropertyDescriptor(obj.foo, 'length');
        print(typeof pd.value, pd.value, pd.writable, pd.enumerable, pd.configurable);
        pd = Object.getOwnPropertyDescriptor(obj.foo, 'prototype');
        print(typeof pd);
        try {
            new obj.foo();
        } catch (e) {
            print(e.name);
        }
    } catch (e) {
        print('computed name for method shorthand failed (not yet supported?)');
        print(e.name);
    }

    // A computed method name is useful in practice for symbol-named functions.

    try {
        obj = eval('({ [Symbol.for("aiee")](a,b) { return a+b; } })');
        print(obj[Symbol.for('aiee')](2, 3));
    } catch (e) {
        print('computed name for method shorthand failed (not yet supported?)');
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
