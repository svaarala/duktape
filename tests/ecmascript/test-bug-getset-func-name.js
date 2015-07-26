/*
 *  Duktape 1.0 would treat a getter/setter in an object literal as a named
 *  function expression, creating a binding for the function (property) name.
 *
 *  The correct behavior seems to be to not create a name binding for the
 *  get/set function.  The function won't be able to refer to itself.  Node
 *  and Rhino agree with this behavior.
 */

/*===
foo getter: number 123
foo setter: number 123
get.name: foo
set.name: foo
===*/

function test() {
    var foo = 123;

    var obj = {
        get foo() {
            print('foo getter:', typeof foo, foo);
            return 'dummy';
        },
        set foo(v) {
            print('foo setter:', typeof foo, foo);
        }
    };

    var dummy = obj.foo;
    obj.foo = 'bar';

    // Check that getter/setter functions still have a 'name' property.
    var pd = Object.getOwnPropertyDescriptor(obj, 'foo');
    print('get.name:', pd.get.name);
    print('set.name:', pd.set.name);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
