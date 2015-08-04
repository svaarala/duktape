/*
 *  Errors thrown by finalizers are silently ignored.  This may be quite
 *  frustrating to track down, so finalizers should as a rule contain a
 *  try-catch block at the very top.
 */

/*---
{
    "custom": true
}
---*/

/*===
obj1 finalizer
WARNING: finalizer failed: ReferenceError
===*/

function test() {
    var obj1, obj2, obj3;

    obj1 = {};
    obj2 = {};
    obj3 = {};

    Duktape.fin(obj1, function (o) {
        // Works.
        print('obj1 finalizer');
    });
    Duktape.fin(obj2, function (o) {
        // The ReferenceError (for 'aiee') is silently eaten and for the
        // user it seems that the finalizer never executed.
        print('obj2 finalizer: ' + aiee);
    });
    Duktape.fin(obj3, function (o) {
        // Try-catch wrapper now catches the ReferenceError.
        try {
            print('obj3 finalizer: ' + aiee);
        } catch (e) {
            print('WARNING: finalizer failed: ' + e.name);
        }
    });
}

try {
    test();
} catch (e) {
    print(e);
}
