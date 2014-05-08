/*
 *  Demonstrate how non-strict arguments 'magic' bindings are kept in sync
 *  with the corresponding identifiers.
 */

/*===
1 1
2 2
2
3 3
4 4
4 3
===*/

function f(x) {
    // Initially, arguments[0] == x == 1.
    print(x, arguments[0]);

    // After 'x = 2', the underlying arguments[0] value is still 1, but
    // 'x' has the value 2.  The underlying value for arguments[0] is
    // no longer in sync with 'x'.
    //
    // However, this is not externally visible.  Looking up arguments[0]
    // prints '2'.  The initial property lookup returns 1, but the
    // exotic [[GetOwnProperty]] behavior overwrites the value with
    // the current value of 'x'.

    x = 2;
    print(x, arguments[0]);

    // Similarly, the overridden value (current value of 'x') is
    // visible through the property descriptor, hiding the discrepancy.
    // The following prints '2'.

    print(Object.getOwnPropertyDescriptor(arguments, "0").value);

    // After this, the underlying arguments[0] value and 'x' have the
    // same value, 3.  The values are again in sync.

    arguments[0] = 3;
    print(x, arguments[0]);

    // Change x again; underlying arguments[0] is out of sync but not
    // externally visible.

    x = 4;
    print(x, arguments[0]);

    // Remove the magic behavior (property map binding) by write protecting
    // the arguments entry.  Because there is no longer any magic behavior,
    // *unsynchronized* old value (3) is now visible.

    Object.defineProperty(arguments, "0", { writable: false });
    print(x, arguments[0]);
}

try {
    f(1);
} catch (e) {
    print(e);
}
