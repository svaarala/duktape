/*---
custom: true
---*/

/*===
sealed
sealed
===*/

var buf;

// Sealing a plain Uint8Array of non-zero size should work because:
// - We must prevent extensions (already non-extensible)
// - Each own property must be made non-configurable (already non-configurable, including both length and indices)
buf = Uint8Array.allocPlain(10);
try {
    Object.seal(buf);
    print('sealed');
} catch (e) {
    print(e.name);
}

// Same thing for a plain Uint8Array of zero size.
buf = Uint8Array.allocPlain(0);
try {
    Object.seal(buf);
    print('sealed');
} catch (e) {
    print(e.name);
}
