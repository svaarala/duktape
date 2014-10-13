/*
 *  Test that finalizers are executed correctly if they are skipped by
 *  one GC round.
 */

/*---
{
    "custom": true
}
---*/

/*===
gc before creating garbage
gc without finalizers
gc with finalizers
finalizer for temp called
done
===*/

function createGarbage() {
    var temp = function tempFunc() {};
    Duktape.fin(temp, function (v) {
        print('finalizer for temp called');
    });

    // Once we return, 'temp' exists only in a reference loop by itself
    // and won't be collected by refcounting.
}

try {
    // Forced GC to ensure GC is "in sync"
    print('gc before creating garbage');
    Duktape.gc(4);

    createGarbage();

    // Mark-and-sweep without finalizers.  (1 << 2) = 4 is a flag from
    // duk_heap.h (this is a fragile dependency):
    //
    // #define DUK_MS_FLAG_NO_FINALIZERS            (1 << 2)   /* don't run finalizers (which may have arbitrary side effects) */

    print('gc without finalizers');
    Duktape.gc(4);

    // Mark-and-sweep with finalizers, should run the pending finalizers.

    print('gc with finalizers');
    Duktape.gc();
} catch (e) {
    print(e.stack || e);
}

print('done');
