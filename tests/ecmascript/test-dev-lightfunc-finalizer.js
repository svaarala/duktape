/*
 *  A lightfunc cannot have a finalizer for two reasons:
 *
 *   (1) Lightfuncs are primitive values and have no refcount, so they are
 *       not finalized like objects.
 *
 *   (2) Even if they were finalized, they cannot store a finalizer reference.
 *       This could be worked around by storing a finalizer reference in
 *       Function.prototype and have that deal with lightfuncs.
 *
 *  This testcase demonstrates that lightfunc finalizers don't work.
 */

/*@include util-object.js@*/

/*---
{
    "custom": true
}
---*/

/*===
true
TypeError
===*/

function lightfuncFinalizerTest() {
    var lfunc = Math.max;

    // Verify built-ins are lightfuncs
    print(valueIsLightFunc(lfunc));

    // Attempt to set a finalizer on the lightfunc directly fails.
    try {
        Duktape.fin(lfunc, function (v) { print('lfunc finalizer'); });
    } catch (e) {
        print(e.name);
        //print(e.stack);
    }

    // Finalizer can be set to Function.prototype but it won't get called
    // because lightfuncs are primitive values without a refcount field.
    Duktape.fin(Function.prototype, function (v) {
        if (valueIsLightFunc(v)) {
            print('inherited finalizer for lightfunc');
        } else {
            //print('inherited finalizer, not for lightfunc: ' + v);
        }
    });

    lfunc = null;
}

try {
    lightfuncFinalizerTest();
} catch (e) {
    print(e.stack || e);
}
