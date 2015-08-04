/*
 *  Constructor calls currently prevent a yield.
 */

/*===
coroutine start
123
TypeError
===*/

var thr;

function Foo() {
    Duktape.Thread.yield(123);
}

function coroutine() {
    print('coroutine start');
    Foo();      // works, not a constructor call
    new Foo();  // not allowed currently -> TypeError
}

try {
    thr = new Duktape.Thread(coroutine);
    print(Duktape.Thread.resume(thr));
    print(Duktape.Thread.resume(thr));
} catch (e) {
    print(e.name);
}
