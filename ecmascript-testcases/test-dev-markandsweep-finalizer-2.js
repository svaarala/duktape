/*
 *  Test Duktape specific mark-and-sweep initiated finalizer calling.
 *  In particular, test that "rescuing" and re-finalizing works
 *  correctly.
 *
 *  Note: print messages for both finalizers are identical because
 *  the order in which they run is not guaranteed.
 *
 *  Manipulation of 'a' and 'b' are in helper functions to guarantee
 *  that no references are held in temporary registers.  Cleanup of
 *  references in temporary registers is currently NOT guaranteed
 *  while a function is running.
 */

/*---
{
    "custom": true
}
---*/

/*===
object
object
finalizer: rescue
finalizer: rescue
object
object
finalizer: rescue
finalizer: rescue
object
object
finalizer: no rescue
finalizer: no rescue
undefined
undefined
===*/

var a;
var b;
var rescue;

function finalizer_a(x) {
    if (rescue) {
        print('finalizer: rescue');
        a = x;
    } else {
        print('finalizer: no rescue');
    }
}

function finalizer_b(x) {
    if (rescue) {
        print('finalizer: rescue');
        b = x;
    } else {
        print('finalizer: no rescue');
    }
}

function init() {
    a = { name: 'a' };
    b = { name: 'b' };

    /* circular refs */
    a.ref = b;
    b.ref = a;

    Duktape.fin(a, finalizer_a);
    Duktape.fin(b, finalizer_b);
}

function delrefs() {
    a = undefined;
    b = undefined;
}

function testrefs() {
    print(typeof a);
    print(typeof b);

    //print(Duktape.refc(a));
    //print(Duktape.refc(b));
}

init();

testrefs();

rescue = true;
delrefs();
Duktape.gc();  // both a and b should be unreachable and have their finalizers executed
Duktape.gc();  // a second gc will rescue the objects, allowing their finalizer to run again
testrefs();

/* Note: a single gc above is NOT sufficient to ensure that the finalizer will run again.
 * Consider the sequence:
 *
 *   1. Value X is made unreachable and GC runs.  GC marks the value as finalizable,
 *      and runs the finalizer.  Finally, GC places value X back to heap_allocated
 *      with the flag FINALIZED.
 *
 *   2. Application code makes X unreachable again before GC runs again.
 *
 *   3. GC detects that X is FINALIZED and not reachable, and will thus not be rescued,
 *      nor will finalization run again.
 *
 * If there is a GC between steps 2 and 3 (as there is here) the value will be rescued
 * and FINALIZED flag will be cleared.  This causes the object to be finalized again in
 * step 3.
 */

rescue = true;
delrefs();
Duktape.gc();  // again
Duktape.gc();
testrefs();

rescue = false;
delrefs();
Duktape.gc();  // swept immediately
testrefs();
