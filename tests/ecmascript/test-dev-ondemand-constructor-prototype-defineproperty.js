/*---
{
    "custom": true
}
---*/

/*===
- Object.defineProperty() and .prototype
TypeError
number 321
number 123
TypeError
done
===*/

// Because the .prototype is intended to be writable, but configurable or
// enumerable, Object.defineProperty() should not allow modification of
// any attributes except: (1) to disable writable, (2) to update the value.
print('- Object.defineProperty() and .prototype');
try {
    // This must fail because the (actually non-existent) .prototype is
    // not configurable.
    var X = function () {};
    Object.defineProperty(X, 'prototype', { enumerable: true });
    print('never here');
} catch (e) {
    print(e.name);
}
try {
    // This must succeed because the .prototype is writable.
    var X = function () {};
    Object.defineProperty(X, 'prototype', { value: 321 });
    print(typeof X.prototype, X.prototype);
} catch (e) {
    print(e);
}
try {
    // Defining all attributes which are compatible with the virtualized
    // existing ones -> must succeed.
    var X = function () {};
    Object.defineProperty(X, 'prototype', {
        value: 123,
        writable: true,
        enumerable: false,
        configurable: false
    });
    print(typeof X.prototype, X.prototype);
} catch (e) {
    print(e);
}
try {
    // Defining all attributes which are incompatible with the virtualized
    // existing ones -> must fail.
    var X = function () {};
    Object.defineProperty(X, 'prototype', {
        value: 123,
        writable: true,
        enumerable: true,  // incompatible
        configurable: false
    });
    print('never here');
} catch (e) {
    print(e.name);
}

print('done');
