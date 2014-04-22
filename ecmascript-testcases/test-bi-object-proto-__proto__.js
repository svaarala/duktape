/*
 *  Object.prototype.__proto__ (ES6)
 */

/*===
__proto__ exists in Object.prototype: true
__proto__ property descriptor: enumerable: false configurable: true set exists: true get exists: true
literal {}: true
literal []: true
before: true 123 undefined
after: true 123 321
TypeError
===*/

function test() {
    var o, pd;
    var a, b;

    // __proto__ existence
    print('__proto__ exists in Object.prototype:', '__proto__' in Object.prototype);

    // Rhino and V8 print true above (Object.prototype.__proto__ exists) but
    // the descriptor here will be undefined for some reason!
    pd = Object.getOwnPropertyDescriptor(Object.prototype, '__proto__');
    if (pd) {
        print('__proto__ property descriptor:', 'enumerable:', pd.enumerable,
              'configurable:', pd.configurable, 'set exists:', (typeof pd.set === 'function'),
              'get exists:', (typeof pd.get === 'function'));
    } else {
        print('__proto__ property descriptor is undefined');
    }

    // A few read checks for built-ins
    o = {};
    print('literal {}:', o.__proto__ === Object.prototype);

    o = [];
    print('literal []:', o.__proto__ === Array.prototype);

    // Setting a prototype
    a = { foo: 123 };
    b = { bar: 321 };
    print('before:', a.__proto__ === Object.prototype, a.foo, a.bar);
    a.__proto__ = b;
    print('after:', a.__proto__ === b, a.foo, a.bar);

    // Attempt to set a prototype loop
    try {
        b.__proto__ = a;
        print('never here');
    } catch (e) {
        // Rhino throws InternalError, ES6 specifies TypeError
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e);
}
