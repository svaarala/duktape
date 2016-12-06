/*
 *  Native function not allowed as an initial function.
 */

/*===
caught error
TypeError
===*/

function test() {
    var T = new Duktape.Thread(Math.cos);

    print(Duktape.Thread.resume(T, 'foo'));
    print(Duktape.Thread.resume(T, 'foo'));
    print(Duktape.Thread.resume(T, 'foo'));
    print(Duktape.Thread.resume(T, 'foo'));
}

try {
    test();
} catch (e) {
    // Print when we get here: if the executor RESUME longjmp handler, we
    // *won't* come here at all.  Rather, the error is propagated out of the
    // executor (as an "internal error").
    print('caught error');
    print(e.name);
}
