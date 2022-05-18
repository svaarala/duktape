/*---
{
    "custom": true
}
---*/

/*===
10
false
10
===*/

// Custom Duktape behavior: we present a virtual, non-configurable .length
// property for typed arrays so the .length delete fails.  Standard behavior
// is for the delete to succeed because no .length property exists; the
// length can still be read however because .length is normally an inherited
// getter which is not affected by the delete.
try {
    var u8 = new Uint8Array(10);
    print(u8.length);
    print(delete u8.length);
} catch (e) {
    print(e.name);
}
print(u8.length);
