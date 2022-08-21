/*
 *  E5.1 Section 10.5, step 5.e: global redeclaration would use [[GetProperty]]
 *  rather than [[GetOwnProperty]] to process global redeclarations.  In ES2015
 *  this was changed to only check for own properties.
 *
 *  This test has been updated to ES2015+ semantics: inherited properties never
 *  prevent global declarations.
 */

var GLOBAL = globalThis || new Function('return this;')();

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

eval("function test_configurable_function() { print('replacement'); }");
test_configurable_function();
Object.prototype.test_configurable_function();

eval("function test_configurable_value() { print('replacement'); }");
test_configurable_value();
print(Object.prototype.test_configurable_value);

eval("function test_configurable_accessor() { print('replacement'); }");
test_configurable_accessor();
tmp = Object.prototype.test_configurable_accessor;
Object.prototype.test_configurable_accessor = 1;

/*===
replacement
321
===*/

eval("function test_nonconfigurable_compatible_value() { print('replacement'); }");
test_nonconfigurable_compatible_value();
print(Object.prototype.test_nonconfigurable_compatible_value);

/*===
replacement
replacement
replacement
true
true
true
1001
1002
getter
setter
===*/

// In ES2015+ none of these prevent redeclaring the global.

eval("function test_nonconfigurable_incompatible_value1() { print('replacement'); }");
test_nonconfigurable_incompatible_value1();

eval("function test_nonconfigurable_incompatible_value2() { print('replacement'); }");
test_nonconfigurable_incompatible_value2();

eval("function test_nonconfigurable_accessor() { print('replacement'); }");
test_nonconfigurable_accessor();

print(Object.getOwnPropertyDescriptor(GLOBAL, 'test_nonconfigurable_incompatible_value1') != null);
print(Object.getOwnPropertyDescriptor(GLOBAL, 'test_nonconfigurable_incompatible_value2') != null);
print(Object.getOwnPropertyDescriptor(GLOBAL, 'test_nonconfigurable_accessor') != null);

print(Object.prototype.test_nonconfigurable_incompatible_value1);
print(Object.prototype.test_nonconfigurable_incompatible_value2);
tmp = Object.prototype.test_nonconfigurable_accessor;
Object.prototype.test_nonconfigurable_accessor = 1;
