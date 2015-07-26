/*
 *  E5.1 Section 10.5, step 5.e.
 *
 *  The global may have an implementation dependent internal prototype.
 *  For us, the prototype is now Object.prototype directly.
 *
 *  The conflicting property in step 5.e may be present in an ancestor,
 *  as is the case when do:
 *
 *    function toString() { ... }
 *
 *  The global object does not contain toString(), but Object.prototype.toString()
 *  will conflict; in other words step 5.c will yield funcAlreadyDeclared = true.
 *
 *  In this case, step 5.e will look at the existing property descriptor found in
 *  the ancestor and will declare a new property in the global object; the ancestor
 *  will never be modified.
 *
 *  Here we test for this case, declaring conflicting functions/values first in
 *  Object.prototype.
 */

var tmp;

Object.defineProperties(Object.prototype, {
    test_configurable_function: {
        value: function() { print('original') },
        writable: false,
        enumerable: false,
        configurable: true
    },
    test_configurable_value: {
        value: 123,
        writable: false,
        enumerable: false,
        configurable: true
    },
    test_configurable_accessor: {
        get: function() { print('getter') },
        set: function() { print('setter') },
        enumerable: false,
        configurable: true
    },
    test_nonconfigurable_compatible_value: {
        // non-configurable, but is not an accessor, and is writable and enumerable
        // (steps 5.e.iii - 5.e.iv)
        value: 321,
        writable: true,
        enumerable: true,
        configurable: false
    },
    test_nonconfigurable_incompatible_value1: {
        value: 1001,
        writable: false,
        enumerable: true,
        configurable: false
    },
    test_nonconfigurable_incompatible_value2: {
        value: 1002,
        writable: true,
        enumerable: false,
        configurable: false
    },
    test_nonconfigurable_accessor: {
        get: function() { print('getter') },
        set: function() { print('setter') },
        enumerable: false,
        configurable: false
    }
});

/*===
replacement
original
replacement
123
replacement
getter
setter
===*/

/* Configurable plain value or accessor: allow redeclaration */

try {
    eval("function test_configurable_function() { print('replacement'); }");
    test_configurable_function();
    Object.prototype.test_configurable_function();
} catch (e) {
    print(e.name);
}

try {
    eval("function test_configurable_value() { print('replacement'); }");
    test_configurable_value();
    print(Object.prototype.test_configurable_value);
} catch (e) {
    print(e.name);
}

try {
    eval("function test_configurable_accessor() { print('replacement'); }");
    test_configurable_accessor();
    tmp = Object.prototype.test_configurable_accessor;
    Object.prototype.test_configurable_accessor = 1;
} catch (e) {
    print(e.name);
}

/*===
replacement
321
===*/

/* Non-configurable compatible value: must be a plain value, and be writable
 * and enumerable.
 */

try {
    eval("function test_nonconfigurable_compatible_value() { print('replacement'); }");
    test_nonconfigurable_compatible_value();
    print(Object.prototype.test_nonconfigurable_compatible_value);
} catch (e) {
    print(e.name);
}

/*===
TypeError
TypeError
TypeError
undefined
undefined
undefined
1001
1002
getter
setter
===*/

/* Non-configurable incompatible values: TypeError and no modification */

try {
    eval("function test_nonconfigurable_incompatible_value1() { print('replacement'); }");
    print('never here');
} catch (e) {
    print(e.name);
}

try {
    eval("function test_nonconfigurable_incompatible_value2() { print('replacement'); }");
    print('never here');
} catch (e) {
    print(e.name);
}

try {
    eval("function test_nonconfigurable_accessor() { print('replacement'); }");
    print('never here');
} catch (e) {
    print(e.name);
}

// verify that nothing was declared in the global object
print(Object.getOwnPropertyDescriptor(this, 'test_nonconfigurable_incompatible_value1'));
print(Object.getOwnPropertyDescriptor(this, 'test_nonconfigurable_incompatible_value2'));
print(Object.getOwnPropertyDescriptor(this, 'test_nonconfigurable_accessor'));

// and that nothing changed in the ancestor
print(Object.prototype.test_nonconfigurable_incompatible_value1);
print(Object.prototype.test_nonconfigurable_incompatible_value2);
tmp = Object.prototype.test_nonconfigurable_accessor;
Object.prototype.test_nonconfigurable_accessor = 1;
