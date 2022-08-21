/*---
custom: true
---*/

/*===
TypeError
false
frozen
true
===*/

var buf;

// Freeze fails because index properties can't be made non-writable
// and non-configurable.
buf = Uint8Array.allocPlain(10);
try {
    Object.freeze(buf);
    print('frozen');
} catch (e) {
    print(e.name);
}
print(Object.isFrozen(buf));

// For a zero-size buffer there are no index properties so freeze
// succeeds.  This matches Uint8Array behavior.
buf = Uint8Array.allocPlain(0);
try {
    Object.freeze(buf);
    print('frozen');
} catch (e) {
    print(e.name);
}
print(Object.isFrozen(buf));
