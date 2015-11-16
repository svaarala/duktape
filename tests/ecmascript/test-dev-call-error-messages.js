/*
 *  Fragile testcase for testing call error messages.
 */

/*---
{
    "custom": true
}
---*/

/*===
TypeError: undefined not callable
TypeError: 123 not callable
TypeError: 321 not callable
TypeError: undefined not callable
TypeError: 234 not callable
"TypeError: undefined not callable"
"TypeError: null not callable"
"TypeError: true not callable"
"TypeError: false not callable"
"TypeError: 123 not callable"
"TypeError: 'a\u1234string' not callable"
"TypeError: [object Array] not callable"
"TypeError: [object Object] not callable"
===*/

var global = 321;

function test() {
    var tmp;
    var dummy = 123;
    var obj = { bar: 234 };

    // Basic cases
    try {
        (undefined)();  // literal
    } catch (e) {
        print(e);
    }
    try {
        dummy();  // register-mapped variable
    } catch (e) {
        print(e);
    }
    try {
        global();  // global variable, slow path lookup
    } catch (e) {
        print(e);
    }
    try {
        obj.foo();  // object property, nonexistent
    } catch (e) {
        print(e);
    }
    try {
        obj.bar();  // object property, exists but not callable
    } catch (e) {
        print(e);
    }

    // Summarization of different value types is already covered by
    // test-dev-prop-error-messages.js, but cover a few values here.

    [
        undefined, null, true, false, 123, 'a\u1234string', [ 1, 2, 3 ], { foo: 'bar' }
    ].forEach(function (v) {
        try {
            v();
        } catch (e) {
            tmp = Duktape.enc('jx', String(e));  // JX encode to get ASCII
            print(tmp);
        }
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
