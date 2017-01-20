/*
 *  If an object is mark-and-sweep finalized and the finalizer breaks the
 *  reference cycle where the object participates, and the object has a zero
 *  refcount after the finalizer returns, the object gets freed immediately.
 */

/*===
gc 1
gc 2, finalizer execution
finalizer called
gc 3, nop
done
===*/

function test() {
    var obj = {};
    obj.ref = {};
    obj.ref.ref = obj;  // Cycle

    Duktape.fin(obj, function (v) {
       print('finalizer called');
       v.ref = null;
    });

    print('gc 1');
    Duktape.gc();
    obj = null;
    print('gc 2, finalizer execution');
    Duktape.gc();
    print('gc 3, nop');
    Duktape.gc();
    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
