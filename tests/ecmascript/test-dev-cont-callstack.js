/*
 *  Test that we can continue after a callstack limit error.
 */

/*---
{
    "custom": true
}
---*/

/*===
RangeError: callstack limit
still here
===*/

function test() {
    // Use an Ecmascript-to-Ecmascript call to hit the call stack limit
    // without hitting the native call limit.  Avoid tail recursion which
    // would cause an infinite loop.

    function f() { f(); return 'dummy'; }

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
