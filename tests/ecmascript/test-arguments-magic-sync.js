/*
 *  Demonstrate how non-strict arguments 'magic' bindings are kept in sync
 *  with the corresponding identifiers.
 */

/*===
1 1 1
2 2
2
3 3 3
4 4 4
4 4 4
5 4 4
5 4 4
5 7 7
===*/

function f(x) {
    // Initially, arguments[0] == x == 1.
    print(x, arguments[0], Object.getOwnPropertyDescriptor(arguments, '0').value);

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
    print(Object.getOwnPropertyDescriptor(arguments, '0').value);

    // After this, the underlying arguments[0] value and 'x' have the
    // same value, 3.  The values are again in sync.
    arguments[0] = 3;
    print(x, arguments[0], Object.getOwnPropertyDescriptor(arguments, '0').value);

    // Change x again; underlying arguments[0] is out of sync but not
    // externally visible.
    x = 4;
    print(x, arguments[0], Object.getOwnPropertyDescriptor(arguments, '0').value);

    // Remove the magic behavior (property map binding) by write protecting
    // the arguments entry.  Revised behavior after ES5.1 is for the value
    // to be synchronized once before write protecting the property.
    Object.defineProperty(arguments, '0', { writable: false });
    print(x, arguments[0], Object.getOwnPropertyDescriptor(arguments, '0').value);

    // No more in sync updates.
    x = 5;
    print(x, arguments[0], Object.getOwnPropertyDescriptor(arguments, '0').value);
    arguments[0] = 6;  // Rejected
    print(x, arguments[0], Object.getOwnPropertyDescriptor(arguments, '0').value);
    Object.defineProperty(arguments, '0', { value: 7 });  // Allowed, configurable (but unmapped)
    print(x, arguments[0], Object.getOwnPropertyDescriptor(arguments, '0').value);
}

f(1);
