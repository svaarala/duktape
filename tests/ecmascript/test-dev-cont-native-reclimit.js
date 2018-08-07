/*
 *  Test that we can continue after a native call recursion limit.
 */

/*---
{
    "custom": true
}
---*/

/*===
RangeError: C stack depth limit
still here
===*/

function test() {
    // Use a native call in the recursion to hit the native recursion
    // limit before call stack limit.

    function f() { [1].map(f); return 'dummy'; }

    try {
        f();
    } catch (e) {
        print(e.name + ': ' + e.message);
    }

    print('still here');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
