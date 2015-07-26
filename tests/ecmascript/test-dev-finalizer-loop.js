/*
 *  Finalizer loop where a finalizer is its own finalizer.
 *  The finalizer will only get called once.
 */

/*===
finalizer called, argument type: function
finished
===*/

function test() {
    function finalizer(obj) {
        print('finalizer called, argument type:', typeof obj);
    }

    Duktape.fin(finalizer, finalizer);

    /* Break the circular reference loop of 'finalizer' by breaking the
     * prototype reference.  However, this is not enough to make the
     * finalizer get collected when returning from this function because
     * the finalizer reference is a reference loop too.
     */
    finalizer.prototype = undefined;
}

try {
    test();
    Duktape.gc();  /* force collection */
    print('finished');
} catch (e) {
    print(e);
}
