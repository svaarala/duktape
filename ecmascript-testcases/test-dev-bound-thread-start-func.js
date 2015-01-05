/*
 *  Bound function as initial function of a Thread.
 */

/*---
{
    "custom": true
}
---*/

/*===
non-bound
func called: 321
bound
func called: 123
===*/

function nonBoundTest() {
    var func = function (x) {
        print('func called:', x);
    };
    var t = new Duktape.Thread(func);
    Duktape.Thread.resume(t, 321);
}

function boundTest() {
    var func = function (x) {
        print('func called:', x);
    };
    var boundFunc = func.bind('myThis', 123);
    var t = new Duktape.Thread(boundFunc);
    Duktape.Thread.resume(t, 321);  // arg ignored because bound
}

try {
    print('non-bound');
    nonBoundTest();

    print('bound');
    boundTest();
} catch (e) {
    print(e.stack || e);
}
