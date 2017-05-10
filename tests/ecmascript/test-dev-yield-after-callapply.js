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
123
===*/

/* Calling via Function.prototype.call() or Function.prototype.apply()
 * no longer prevents a yield in Duktape 2.2.
 */

function innerfunc() {
    Duktape.Thread.yield(123);
}

function coroutine1() {
    innerfunc.call();
}

function coroutine2() {
    innerfunc.apply();
}

function coroutine3() {
    Reflect.apply(innerfunc);
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

try {
    thread = new Duktape.Thread(coroutine3);
    res = Duktape.Thread.resume(thread, 0);
    print(res);
} catch (e) {
    print(e.name);
}
