/*
 *  Reflect.defineProperty()
 */

/*===
basic operation
true
quux
pigz r dum
pigz r dum true true true
true
true
foo
false
foo
foo false false false
===*/

function basicTest() {
    'use strict';

    var obj = { prop1: "baz" };
    var desc;

    // DefProp of existing property with no attributes leaves existing
    // attributes as is
    print(Reflect.defineProperty(obj, 'prop1', { value: "quux" }));
    print(obj.prop1);
    obj.prop1 = "pigz r dum";  // should succeed
    print(obj.prop1);
    desc = Reflect.getOwnPropertyDescriptor(obj, 'prop1');
    print(desc.value, desc.writable, desc.enumerable, desc.configurable);
    print(Reflect.defineProperty(obj, 'prop1', { value: "baz" }));

    // DefProp of new property with no attributes uses defaults (i.e. all false)
    print(Reflect.defineProperty(obj, 'prop2', { value: "foo" }));
    print(obj.prop2);
    print(Reflect.defineProperty(obj, 'prop2', { value: "bar" }));  // should fail
    print(obj.prop2);
    var desc = Reflect.getOwnPropertyDescriptor(obj, 'prop2');
    print(desc.value, desc.writable, desc.enumerable, desc.configurable);
}

try {
    print("basic operation");
    basicTest();
} catch (e) {
    print(e.stack || e);
}

/*===
writable
foo
foo false true true
TypeError
foo
bar
bar true true true
===*/

function writableTest() {
    'use strict';

    var obj = { prop: "foo" };
    var desc;

    Reflect.defineProperty(obj, 'prop', { writable: false });
    print(obj.prop);
    desc = Reflect.getOwnPropertyDescriptor(obj, 'prop');
    print(desc.value, desc.writable, desc.enumerable, desc.configurable);
    try {
        obj.prop = "bar";  // should fail
        print("never here");
    } catch (e) {
        print(e.name);
    }
    print(obj.prop);
    Reflect.defineProperty(obj, 'prop', { writable: true });
    obj.prop = "bar";
    print(obj.prop);
    desc = Reflect.getOwnPropertyDescriptor(obj, 'prop');
    print(desc.value, desc.writable, desc.enumerable, desc.configurable);
}

try {
    print("writable");
    writableTest();
} catch (e) {
    print(e.stack || e);
}

/*===
enumerable
true
true
foo true true true
bar true false true
prop1
true
bar true true true
prop1,prop2
===*/

function enumerableTest() {
    'use strict';

    var obj = { prop1: "foo", prop2: "bar" };
    var desc;

    print(Reflect.defineProperty(obj, 'prop1', { enumerable: true }));
    print(Reflect.defineProperty(obj, 'prop2', { enumerable: false }));

    desc = Reflect.getOwnPropertyDescriptor(obj, 'prop1');
    print(desc.value, desc.writable, desc.enumerable, desc.configurable);
    desc = Reflect.getOwnPropertyDescriptor(obj, 'prop2');
    print(desc.value, desc.writable, desc.enumerable, desc.configurable);
    print(Object.keys(obj));

    print(Reflect.defineProperty(obj, 'prop2', { enumerable: true }));
    desc = Reflect.getOwnPropertyDescriptor(obj, 'prop2');
    print(desc.value, desc.writable, desc.enumerable, desc.configurable);
    print(Object.keys(obj));
}

try {
    print("enumerable");
    enumerableTest();
} catch (e) {
    print(e.stack || e);
}

/*===
configurable
foo true true false
bar
true
bar false true false
false
bar false true false
===*/

function configurableTest() {
    'use strict';

    var obj = { prop: "foo" };
    var desc;

    Reflect.defineProperty(obj, 'prop', { configurable: false });

    desc = Reflect.getOwnPropertyDescriptor(obj, 'prop');
    print(desc.value, desc.writable, desc.enumerable, desc.configurable);
    obj.prop = "bar";  // still writable, so should succeed
    print(obj.prop);
    print(Reflect.defineProperty(obj, 'prop', { writable: false }));  // this is allowed
    desc = Reflect.getOwnPropertyDescriptor(obj, 'prop');
    print(desc.value, desc.writable, desc.enumerable, desc.configurable);
    print(Reflect.defineProperty(obj, 'prop', { writable: true }));  // should fail
    desc = Reflect.getOwnPropertyDescriptor(obj, 'prop');
    print(desc.value, desc.writable, desc.enumerable, desc.configurable);
}

try {
    print("configurable");
    configurableTest();
} catch (e) {
    print(e.stack || e);
}
