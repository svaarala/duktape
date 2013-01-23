/*
 *  Constructor calls currently prevent a yield.
 */

/*===
123
TypeError
===*/

var thr;

function Foo() {
    __duk__.yield(123);
}

function coroutine() {
    print('coroutine start');
    Foo();      // works, not a constructor call
    new Foo();  // not allowed currently -> TypeError
}

try {
    thr = __duk__.spawn(coroutine);
    print(__duk__.resume(thr));
    print(__duk__.resume(thr));
} catch (e) {
    print(e.name);
}
