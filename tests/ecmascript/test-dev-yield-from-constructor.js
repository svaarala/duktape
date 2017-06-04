/*
 *  Constructor calls prevent a yield until Duktape 2.2.
 *  Test for Duktape 2.2 behavior.
 */

/*===
coroutine start
123
123
coroutine end
321
===*/

var thr;

function Foo() {
    Duktape.Thread.yield(123);
}

function coroutine() {
    print('coroutine start');
    Foo();
    new Foo();
    print('coroutine end');
    return 321;
}

try {
    thr = new Duktape.Thread(coroutine);
    print(Duktape.Thread.resume(thr));
    print(Duktape.Thread.resume(thr));
    print(Duktape.Thread.resume(thr));
} catch (e) {
    print(e.name);
}
