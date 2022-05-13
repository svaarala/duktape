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
    print(foo.hasOwnProperty('caller'));
} catch (e) {
    print(e.name);
}

try {
    print(foo.caller);
} catch (e) {
    print(e.name);
}

try {
    print(bar.hasOwnProperty('caller'));
} catch (e) {
    print(e.name);
}

try {
    print(bar.caller);
} catch (e) {
    print(e.name);
}

try {
    print(Object.getPrototypeOf(bar).hasOwnProperty('caller'));
    print(typeof Object.getOwnPropertyDescriptor(Object.getPrototypeOf(bar), 'caller').get === 'function');
    print(typeof Object.getOwnPropertyDescriptor(Object.getPrototypeOf(bar), 'caller').set === 'function');
} catch (e) {
    print(e.name);
}
