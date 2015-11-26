/*
 *  Property assignment expression cases where the key expression mutates
 *  the variable holding the base value.
 */

/*===
setter called: bar
name,foo,redirect,bar
name
name,foo,bar
name
name,bar
name
name,bar
name
foo
bar
===*/

function test1() {
    var obj1 = { name: 'obj1' };
    var obj2 = { name: 'obj2' };
    var obj1orig = obj1;

    // Accessor in property key swaps reference in outer scope.
    // Works incorrectly in Duktape 1.3.0.
    Object.defineProperty(obj1, 'foo', {
        set: function (v) {
            print('setter called:', v);
            obj1.redirect = v;
            obj1 = obj2;
        }
    });

    // The outermost 'obj1' must be evaluated to an immutable value (temporary
    // or constant) before evaluating the rest, because it may get assigned to.
    obj1[obj1['foo'] = 'bar'] = 'quux';

    print(Object.getOwnPropertyNames(obj1orig));
    print(Object.getOwnPropertyNames(obj2));
}

function test2() {
    var obj1 = { name: 'obj1' };
    var obj2 = { name: 'obj2' };
    var obj1orig = obj1;

    // Same effect using comma expression.
    // Works incorrectly in Duktape 1.3.0.
    obj1[(obj1['foo'] = 'bar'), (obj1 = obj2), 'bar'] = 'quux';

    print(Object.getOwnPropertyNames(obj1orig));
    print(Object.getOwnPropertyNames(obj2));
}

function test3() {
    var obj1 = { name: 'obj1' };
    var obj2 = { name: 'obj2' };
    var obj1orig = obj1;

    // Same effect using comma expression on the RHS.  Works correctly
    // in Duktape 1.3.0.
    obj1.bar = ((obj1 = obj2), 'quux');

    print(Object.getOwnPropertyNames(obj1orig));
    print(Object.getOwnPropertyNames(obj2));
}

function test4() {
    var obj1 = { name: 'obj1' };
    var obj2 = { name: 'obj2' };
    var obj1orig = obj1;

    // Same effect using comma expression on the RHS, but using a different
    // property access syntax.  Works correctly in Duktape 1.3.0.
    obj1['bar'] = ((obj1 = obj2), 'quux');

    print(Object.getOwnPropertyNames(obj1orig));
    print(Object.getOwnPropertyNames(obj2));
}

function test5() {
    var obj1 = { name: 'obj1' };

    // Avoiding an explicit copy from a reg-bound variable to a temporary
    // would be nice for at least these common cases.
    obj1.name = 'foo';
    print(obj1.name);
    obj1['name'] = 'bar';
    print(obj1.name);
}

try {
    test1();
    test2();
    test3();
    test4();
    test5();
} catch (e) {
    print(e.stack || e);
}
