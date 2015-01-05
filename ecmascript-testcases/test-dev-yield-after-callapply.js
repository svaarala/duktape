/*---
{
    "custom": true
}
---*/

var thread;
var res;

/*===
123
123
===*/

/* Calling via Function.prototype.call() or Function.prototype.apply()
 * currently prevents a yield.
 */

function innerfunc() {
    Duktape.Thread.yield(123);
}

function coroutine1() {
    // This is a native call so the current (naive) handling prevents a later yield
    innerfunc.call();
}

function coroutine2() {
    // Same here
    innerfunc.apply();
}

try {
    thread = new Duktape.Thread(coroutine1);
    res = Duktape.Thread.resume(thread, 0);
    print(res);
} catch (e) {
    print(e.name);
}

try {
    thread = new Duktape.Thread(coroutine2);
    res = Duktape.Thread.resume(thread, 0);
    print(res);
} catch (e) {
    print(e.name);
}
