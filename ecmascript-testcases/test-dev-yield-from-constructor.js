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
    __duk__.Thread.yield(123);
}

function coroutine() {
    print('coroutine start');
    Foo();      // works, not a constructor call
    new Foo();  // not allowed currently -> TypeError
}

try {
    thr = new __duk__.Thread(coroutine);
    print(__duk__.Thread.resume(thr));
    print(__duk__.Thread.resume(thr));
} catch (e) {
    print(e.name);
}
