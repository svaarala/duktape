/*
 *  Yielding from eval code is currently not allowed and should
 *  cause a clean TypeError.
 */

/*===
TypeError
===*/

function test_eval() {
    var t = new Duktape.Thread(function() {
        eval('Duktape.Thread.yield(1234)');
    });
    Duktape.Thread.resume(t)
}

try {
    test_eval();
} catch (e) {
    print(e.name);
}
