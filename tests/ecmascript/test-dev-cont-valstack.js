/*
 *  Test that we can continue after a valstack limit error.
 */

/*---
{
    "custom": true
}
---*/

/*===
RangeError: valstack limit
still here
===*/

function test() {
    // Use an Ecmascript-to-Ecmascript call to hit the value stack limit
    // without hitting the native call limit.  Use a function with a lot
    // of temporaries to ensure value stack limit is reached before call
    // stack limit (this depends on specific constants of course).  Avoid
    // tail recursion which would cause an infinite loop.

    var src = [];
    var i;

    src.push('(function test() {');
    for (i = 0; i < 1e4; i++) {
        src.push('var x' + i + ' = ' + i + ';');
    }
    src.push('var t = test(); return "dummy"; })');
    src = src.join('');

    var f = eval(src);

    try {
        f();
    } catch (e) {
        print(e.name + ': ' + e.message);
    }

    print('still here');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
