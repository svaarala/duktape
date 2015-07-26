/*
 *  Named function expressions are in an internal reference loop which prevents
 *  refcount based collection.  This testcase illustrates the difference between
 *  named and anonymous function expressions at the moment.
 */

/*---
{
    "custom": true
}
---*/

/*===
anon refcount: 3
anon refcount: 2
anon refcount: 2
assign null
finalized
force gc
named refcount: 4
named refcount: 3
named refcount: 3
assign null
typeof named: undefined
force gc
finalized
===*/

function anonTest() {
    var f = function() {};

    // Print refcount, expected is 3: variable, call argument,
    // f_anon.prototype.constructor.
    print('anon refcount:', Duktape.info(f)[2]);

    // Break ref loop, should decrease refcount by 1.
    f.prototype.constructor = null;
    print('anon refcount:', Duktape.info(f)[2]);

    // Set finalizer, no effect on refcount (function points to
    // finalizer, not vice versa).
    Duktape.fin(f, function () { print('finalized'); });
    print('anon refcount:', Duktape.info(f)[2]);

    // Should refcount collect here.
    print('assign null');
    f = null;

    // Should collect at least here.
    print('force gc');
    Duktape.gc();
}

function namedTest() {
    var f = function named() {};

    // Print refcount, expected is 4: variable, call argument,
    // f_anon.prototype.constructor, and internal environment
    // record holding the "named"->func binding
    print('named refcount:', Duktape.info(f)[2]);

    // Break prototype reference loop, should decrease refcount by 1.
    // Doesn't break the scope reference loop: the function points
    // to the environment record, and the environment record points
    // to the function.
    f.prototype.constructor = null;
    print('named refcount:', Duktape.info(f)[2]);

    // Set finalizer, no effect on refcount (function points to
    // finalizer, not vice versa).
    Duktape.fin(f, function () { print('finalized'); });
    print('named refcount:', Duktape.info(f)[2]);

    // Ideally would refcount collect here, but the environment record
    // reference loop prevents this from happening.  This should be fixed
    // at some point but the fix is not trivial.
    print('assign null');
    f = null;

    // Just a sanity check, 'named' is only visible inside f(), so
    // it should be undefined here.
    print('typeof named:', typeof named);

    // Should collect here.
    print('force gc');
    Duktape.gc();
}

try {
    anonTest();
} catch (e) {
    print(e);
}

try {
    namedTest();
} catch (e) {
    print(e);
}
