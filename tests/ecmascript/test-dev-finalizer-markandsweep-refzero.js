/*
 *  Special case in finalization:
 *
 *  - Object in reference cycle is queued for finalization by mark-and-sweep.
 *  - Finalizer is executed, FINALIZED is set, object is queued back to
 *    heap_allocated.
 *  - While waiting for the next mark-and-sweep round to make a rescue/free
 *    decision, the object's refcount drops to zero.
 *
 *  Current handling (Duktape 2.1) is to detect the situation in REFZERO
 *  handling and free the object without going through finalize_list and
 *  mark-and-sweep unnecessarily.  Prior to 2.1, the object would be queued
 *  back for a new mark-and-sweep round.
 */

/*===
gc 1
gc 2, finalizer
finalizer called
call func()
func called
set func to null
gc 3, nop
done
===*/

function test() {
    var obj = {};
    obj.ref = {};
    obj.ref.ref = obj;  // cycle

    var func;

    Duktape.fin(obj, function (v) {
        print('finalizer called');
        // When finalizer finishes the object is reachable via 'func'.
        // When func() is called and set to null, it gets a REFZERO.
        func = function () {
            print('func called');
            v.ref = null;  // break cycle
        };
        func.prototype = null;  // break cycle
    });

    print('gc 1');
    Duktape.gc();
    obj = null;
    print('gc 2, finalizer');
    Duktape.gc();  // finalizer execution
    print('call func()');
    func();
    print('set func to null');
    func = null;   // DECREF
    print('gc 3, nop');
    Duktape.gc();  // should no longer see object
    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
