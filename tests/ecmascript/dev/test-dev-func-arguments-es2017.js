/*===
false
TypeError
false
TypeError
true
true
true
===*/

function foo() {}
function bar() { 'use strict'; }

// For non-strict functions difference to V8: in V8 there is an own property with
// 'null' value, with Duktape 3.x there's no own property and attempt to read is
// an error from inherited getter.
try {
    print(foo.hasOwnProperty('arguments'));
} catch (e) {
    print(e.name);
}

try {
    print(foo.arguments);
} catch (e) {
    print(e.name);
}

try {
    print(bar.hasOwnProperty('arguments'));
} catch (e) {
    print(e.name);
}

try {
    print(bar.arguments);
} catch (e) {
    print(e.name);
}

try {
    print(Object.getPrototypeOf(bar).hasOwnProperty('arguments'));
    print(typeof Object.getOwnPropertyDescriptor(Object.getPrototypeOf(bar), 'arguments').get === 'function');
    print(typeof Object.getOwnPropertyDescriptor(Object.getPrototypeOf(bar), 'arguments').set === 'function');
} catch (e) {
    print(e.name);
}
