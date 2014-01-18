/*
 *  Normal Ecmascript functions declared by user code (and all Duktape/C)
 *  functions are constructable.  Also built-in top level functions like
 *  Number, String, etc. are constructable.  The rest of the built-in
 *  functions apparently must not be.  Test a few here (not comprehensive).
 */

/*---
{
    "custom": true
}
---*/

/*===
TypeError
constructable
constructable
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
===*/

function testConstruct(fn) {
    try {
        new fn();
        print('constructable');
    } catch (e) {
        print(e.name);
    }
}

try {
    testConstruct(this);
    testConstruct(Number);
    testConstruct(String);
    testConstruct(Math);
    testConstruct(JSON);
    testConstruct(Math.cos);
    testConstruct(String.prototype.toUpperCase);

    // Duktape specific: stack getters are not constructable
    testConstruct(Object.getOwnPropertyDescriptor(Error.prototype, 'stack').get);
    testConstruct(Object.getOwnPropertyDescriptor(Error.prototype, 'stack').set);
} catch (e) {
    print(e);
}
