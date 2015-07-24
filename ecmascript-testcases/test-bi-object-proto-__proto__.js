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
before: true 123 undefined
after: true 123 321
TypeError 123 321 undefined 321
before: true 123 undefined function
after: false 123 undefined undefined
__proto__ in a: false
undefined 123 321
boolean 123 321
boolean 123 321
number 123 321
string 123 321
undefined TypeError
null TypeError
boolean undefined undefined
boolean undefined undefined
number undefined undefined
string undefined undefined
object 123 321
===*/

function test() {
    var o, pd;
    var a, b;
    var getter;

    // __proto__ existence
    print('__proto__ exists in Object.prototype:', '__proto__' in Object.prototype);

    // Rhino and V8 print true above (Object.prototype.__proto__ exists) but
    // the descriptor here will be undefined for some reason!
    // ES6 requires that the property be configurable but not enumerable.
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
        a = { foo: 123 };
        b = { bar: 321 };
        print('before:', a.__proto__ === Object.prototype, a.foo, a.bar);
        a.__proto__ = b;
        print('after:', a.__proto__ === b, a.foo, a.bar);
        b.__proto__ = a;
        print('never here');
    } catch (e) {
        // Rhino throws InternalError, ES6 specifies TypeError
        print(e.name, a.foo, a.bar, b.foo, b.bar);
    }

    // Setting a prototype to null.
    // NOTE: evaluating a.__proto__ afterwards yields undefined (instead of null):
    // 'a' no longer inherits from Object.prototype and thus has no __proto__
    // accessor property.  This is the ES6 behavior right now, but e.g. Rhino
    // disagrees.
    a = { foo: 123 };
    print('before:', a.__proto__ === Object.prototype, a.foo, a.bar, typeof a.toString);
    a.__proto__ = null;
    print('after:', a.__proto__ === null, a.foo, a.bar, typeof a.toString);
    print('__proto__ in a:', '__proto__' in a);  // false in Duktape/ES6, true in Rhino/V8

    // Attempt to set prototype to something else than null/object:
    // ES6: ignore silently
    [ undefined, true, false, 123, 'foo' ].forEach(function (x) {
        try {
            a = { foo: 123 };
            b = { bar: 321 };
            a.__proto__ = b;
            a.__proto__ = x;  // ignored without changing prototype, so 'b' remains as prototype
            print(x === null ? 'null' : typeof x, a.foo, a.bar);
        } catch (e) {
            print(x === null ? 'null' : typeof x, e.name);
        }
    });

    // Attempt to set prototype with 'this' binding not an object (call setter directly):
    // ES6: TypeError for undefined and null (not object coercible), ignore for others
    pd = Object.getOwnPropertyDescriptor(Object.prototype, '__proto__');
    setter = pd ? pd.set : null;
    if (setter) {
        a = { foo: 123 };
        [ undefined, null, true, false, 123, 'foo', a ].forEach(function (x) {
            try {
                setter.call(x, { bar: 321 });
                print(x === null ? 'null' : typeof x, x.foo, x.bar);
            } catch (e) {
                print(x === null ? 'null' : typeof x, e.name);
            }
        });
    } else {
        print('no setter');
    }

    // TODO: coercion order tests (side effect / error message if multiple errors)
}

try {
    test();
} catch (e) {
    print(e);
}
