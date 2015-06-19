/*
 *  Execution timeout: tailcall function
 */

/*---
{
    "skip": true
}
---*/

// See: GH-214
function f() {
    return f();
}

function test() {
    f();
}

try {
    test();
} catch (e) {
    print(e.name);
}
