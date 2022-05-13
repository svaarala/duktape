/*===
test 1, step 1
fin1
test 1, step 2
test 1, step 3
test 1, step 4
test 2, step 1
test 2, step 2
test 2, step 3
test 2, step 4
test 3, step 1
fin3
test 3, step 2
test 3, step 3
test 3, step 4
===*/

function test() {
    /* Base case: normal object finalization. */
    var O = {};
    Duktape.fin(O, function () { print('fin1'); });
    print('test 1, step 1');
    O = null;
    print('test 1, step 2');
    Duktape.gc();
    print('test 1, step 3');
    Duktape.gc();
    print('test 1, step 4');

    /* Proxy finalization, finalizer set on Proxy.  Under the hood
     * uses duk_set_finalizer() on the proxy.  This now has the behavior
     * where:
     *    1) the _Finalizer is set on the target, but
     *    2) the "have finalizer" flag is set on the Proxy
     *
     * As a result the finalizer doesn't run at all.
     */
    var P = (function () { return new Proxy({}, {}); })();
    Duktape.fin(P, function () { print('fin2'); });
    print('test 2, step 1');
    P = null;
    print('test 2, step 2');
    Duktape.gc();
    print('test 2, step 3');
    Duktape.gc();
    print('test 2, step 4');

    /* Proxy finalization, finalizer set on target explicitly. */
    var T = {};
    Duktape.fin(T, function () { print('fin3'); });
    var P = (function () { return new Proxy(T, {}); })();
    T = null;
    print('test 3, step 1');
    P = null;
    print('test 3, step 2');
    Duktape.gc();
    print('test 3, step 3');
    Duktape.gc();
    print('test 3, step 4');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
