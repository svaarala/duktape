/*
 *  https://github.com/svaarala/duktape/issues/2030
 */

/*===
done
===*/

Duktape.fin(
    Proxy, function( ) {
        function f( ) {
            performance ('g called', isFinite(Error), TypeError(g));
        }
        Duktape.Thread.resume(new Duktape.Thread(f));
});

print('done');
