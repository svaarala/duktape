/*
 *  Unwind and thr->ptr_curr_pc related bug when developing GH-294.
 */

/*===
returning
obj finalized
returning
obj finalized
done
===*/

function finalizer() {
    print('obj finalized');
}

function func() {
    var obj = {};
    Duktape.fin(obj, finalizer);

    print('returning');
}

function test() {
    /*
     *  During development of GH-294 there was a bug in thr->ptr_curr_pc
     *  handling: thr->ptr_curr_pc was not NULLed before a longjmp (it was
     *  synced though) with the assumption that the unwind path would never
     *  touch it and it would therefore be harmless.
     *
     *  However, if an unwind side effect (such as a finalizer call) causes
     *  a function call, the call handling will use thr->ptr_curr_pc to sync
     *  the current PC, but the topmost activation has changed.  This leads
     *  to the incorrect activation's curr_pc being updated.
     */

    func();
    func();
}

try {
    test();
    print('done');
} catch (e) {
    print(e.stack || e);
}
