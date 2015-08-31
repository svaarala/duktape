/*
 *  Illustrate a bug in debugger handling of side effect triggered function
 *  calls, see https://github.com/svaarala/duktape/issues/303.
 *
 *  You'll need to debug through this test manually to see the issue, as there
 *  are no automatic debugger regression tests at the moment (2015-08-31).
 */

/*===
get
get
get
get
get
get
get
get
get
get
===*/

var obj = {};
Object.defineProperty(obj, 'prop', {
    get: function () { print('get'); }
});

function test() {
    // Step into test(), then step into the getter.  Set a breakpoint on one
    // of the lines below and Resume.  The breakpoint will be skipped in
    // Duktape 1.3.  The same thing happens if you e.g. StepOut from the
    // getter.

    obj.prop;
    obj.prop;
    obj.prop;
    obj.prop;
    obj.prop;
    obj.prop;
    obj.prop;
    obj.prop;
    obj.prop;
    obj.prop;
}

test();
