/*===
a called
a called
b called
b called
===*/

/*---
{
    "use_strict": true
}
---*/

'use strict';

// Duktape 2.5.0 and prior reject strict mode function statements.
// Duktape >2.5.0 allows them to improve compatibility with existing
// code base.
try {
    function a() {
        print('a called');
    }
    a();
} catch (e) {
    print(e.stack || e);
}
// In Node.js v12.7.0 'a' is not visible here, i.e. 'a' has block visibility
// (ReferenceError here).  Duktape 2.x uses hoist semantics.
try {
    a();
} catch (e) {
    print(e.name);
}

function test() {
    try {
        function b() {
            print('b called');
        }
        b();
    } catch (e) {
        print(e.stack || e);
    }
    // Same here.
    try {
        b();
    } catch (e) {
        print(e.name);
    }
}
test();
