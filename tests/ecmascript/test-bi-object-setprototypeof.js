/*
 *  Object.setPrototypeOf() (ES6)
 */

/*===
Object.setPrototypeOf exists true
function
before: true 123 undefined
after: true 123 321
before: true 123 undefined
after: true 123 321
TypeError 123 321 undefined 321
before: true 123 undefined function
after: true 123 undefined undefined
undefined TypeError
boolean TypeError
boolean TypeError
number TypeError
string TypeError
undefined TypeError
null TypeError
boolean true undefined undefined
boolean true undefined undefined
number true undefined undefined
string true undefined undefined
object true 123 321
===*/

function test() {
    var o, pd;
    var a, b;
    var ret;

    // Method existence
    print('Object.setPrototypeOf exists', 'setPrototypeOf' in Object);
    print(typeof Object.setPrototypeOf);

    // Setting a prototype
    a = { foo: 123 };
    b = { bar: 321 };
    print('before:', Object.getPrototypeOf(a) === Object.prototype, a.foo, a.bar);
    Object.setPrototypeOf(a, b);
    print('after:', Object.getPrototypeOf(a) === b, a.foo, a.bar);

    // Attempt to set a prototype loop
    try {
        a = { foo: 123 };
        b = { bar: 321 };
        print('before:', Object.getPrototypeOf(a) === Object.prototype, a.foo, a.bar);
        Object.setPrototypeOf(a, b);
        print('after:', Object.getPrototypeOf(a) === b, a.foo, a.bar);
        Object.setPrototypeOf(b, a);  // error, no change in b's prototype
        print('never here');
    } catch (e) {
        print(e.name, a.foo, a.bar, b.foo, b.bar);
    }

    // Setting a prototype to null
    a = { foo: 123 };
    print('before:', Object.getPrototypeOf(a) === Object.prototype, a.foo, a.bar, typeof a.toString);
    Object.setPrototypeOf(a, null);
    print('after:', Object.getPrototypeOf(a) === null, a.foo, a.bar, typeof a.toString);

    // Attempt to set prototype to something else than null/object
    // ES6: TypeError when using setPrototypeOf, ignored when using __proto__ (!)
    [ undefined, true, false, 123, 'foo' ].forEach(function (x) {
        try {
            a = { foo: 123 };
            b = { bar: 321 };
            Object.setPrototypeOf(a, b);
            Object.setPrototypeOf(a, x);  // TypeError, 'b' remains as prototype
            print(x === null ? 'null' : typeof x, a.foo, a.bar);
        } catch (e) {
            print(x === null ? 'null' : typeof x, e.name);
        }
    });

    // Attempt to set prototype of something other than an object
    // ES6: TypeError for undefined and null (not object coercible), value is returned as is
    a = { foo: 123 };
    [ undefined, null, true, false, 123, 'foo', a ].forEach(function (x) {
        try {
            ret = Object.setPrototypeOf(x, { bar: 321 });
            print(x === null ? 'null' : typeof x, ret === x, x.foo, x.bar);
        } catch (e) {
            print(x === null ? 'null' : typeof x, e.name);
        }
    });

    // TODO: coercion order tests (side effect / error message if multiple errors)
}

try {
    test();
} catch (e) {
    print(e);
}
