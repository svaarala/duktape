/*
 *  Identifier reference (E5 Section 11.1.2).
 */

/*===
Infinity
undefined
var 1
var 2
===*/

/* global identifier */
print(Infinity);
print(undefined);

/* var1 defined into global object ("with"-like object binding) */
var1 = "var 1";
print(var1);

/* var2 defined explicitly into global object */
var var2 = "var 2";
print(var2);

/*===
var 1
234
123
[object Function]
123
===*/

function test(arg1) {
    var foo = 123;
    print(var1);  // access global identifier
    print(arg1);  // access argument
    print(foo);   // access local variable
    function inner() {};
    print(Object.prototype.toString.call(inner));  // access local function declaration

    (function () { print(foo); })();  // access outer function variable
}

test(234);
