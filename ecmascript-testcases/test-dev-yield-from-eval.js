/*
 *  Yielding from eval code is currently not allowed and should
 *  cause a clean TypeError.
 */

/*===
TypeError
===*/

function test_eval() {
    var t = new __duk__.Thread(function() {
        eval('__duk__.Thread.yield(1234)');
    });
    __duk__.Thread.resume(t)
}

try {
    test_eval();
} catch (e) {
    print(e.name);
}

