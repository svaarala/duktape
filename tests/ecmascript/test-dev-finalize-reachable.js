/*
 *  Duktape 0.10.0 finalizes even reachable objects when a heap is
 *  destroyed, to give native code the chance to free resources
 *  reliably.
 */

/*---
{
    "custom": true
}
---*/

/*===
finalizer executed
===*/

this.obj = {};
Duktape.fin(this.obj, function () {
    print('finalizer executed');
});

// this.obj is reachable, finish
