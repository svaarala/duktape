
var thread;
var res;

/*===
123
===*/

/* Calling via Function.prototype.call() or Function.prototype.apply() would
 * prevent a yield().
 */

function innerfunc() {
    __duk__.yield(123);
}

function coroutine1() {
    // This is a native call so naive handling would prevent a later yield
    innerfunc.call();
}

function coroutine2() {
    // Same here
    innerfunc.apply();
}

try {
    thread = __duk__.spawn(coroutine1);
    res = __duk__.resume(thread, 0);
    print(res);
} catch (e) {
    print(e.name);
}

try {
    thread = __duk__.spawn(coroutine2);
    res = __duk__.resume(thread, 0);
    print(res);
} catch (e) {
    print(e.name);
}

