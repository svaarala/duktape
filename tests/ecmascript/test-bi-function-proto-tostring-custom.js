/*
 *  Duktape custom behavior for Function.prototype.toString().
 */

/*---
{
    "custom": true
}
---*/

/*===
function foo() { [ecmascript code] }
function cos() { [native code] }
function bound foo() { [bound code] }
function bound cos() { [bound code] }
===*/

function test() {
    var scriptFunc = function foo() {};
    var nativeFunc = Math.cos;
    var boundFunc1 = scriptFunc.bind(null, 1);
    var boundFunc2 = nativeFunc.bind(null, 1);

    [ scriptFunc, nativeFunc, boundFunc1, boundFunc2 ].forEach(function (v) {
        print(String(v));
    });

    // Lightfunc and some other cases are covered by
    // tests/api/test-dev-func-tostring.c.
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
