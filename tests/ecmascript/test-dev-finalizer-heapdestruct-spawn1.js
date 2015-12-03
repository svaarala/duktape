/*
 *  Test heap destruction finalizer sanity limit for "runaway finalizers".
 *  Here, test a finalizer which spawns one new finalizable object which
 *  does the same.
 *
 *  This testcase is fragile: the number of finalizations depends on internal
 *  parameters which may be changed without breaking compatibility promises.
 *  The important thing is that the process terminates in a reasonable time.
 */

/*---
{
    "custom": true
}
---*/

/*===
object true
object true
object true
object true
object true
object true
object true
object true
object true
object true
object true
object true
object true
object true
object true
object true
object true
object true
object true
object true
object true
===*/

function mkObj() {
    var obj = {};
    Duktape.fin(obj, function myFinalizer(o, heapDestruct) {
        print(typeof o, heapDestruct);
        var dummy = mkObj();
    });
    return obj;
}

var ref = mkObj();

// Let heap destruction handle the finalizable objects.
