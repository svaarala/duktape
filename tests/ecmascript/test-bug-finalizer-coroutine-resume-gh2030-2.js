/*
 *  https://github.com/svaarala/duktape/issues/2030
 *
 *  Testcase adapted a bit in this version.
 */

/*===
done
finalizer called
f called
===*/

Duktape.fin(Proxy, function( ) {
    function f() {
        print('f called');
        performance();  // Throws.
    }
    print('finalizer called');
    Duktape.Thread.resume(new Duktape.Thread(f));
});

print('done');
