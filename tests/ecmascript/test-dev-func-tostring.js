/*
 *  Test the updated Function .toString() format in Duktape 1.5.0.
 */

/*---
{
    "custom": true
}
---*/

/*===
function () {"ecmascript"}
function foo() {"ecmascript"}
function cos() {"native"}
===*/

function test() {
    var fn;

    // Anonymous function
    fn = function (){};
    print(fn);

    // Named function
    fn = function foo(){};
    print(fn);

    // Native function
    fn = Math.cos;
    print(fn);

    // Lightfunc and some other cases are covered by
    // tests/api/test-dev-func-tostring.c.
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
