/*
 *  Check that built-in Duktape/C functions inherit directly from
 *  Function.prototype (not %NativeFunctionPrototype introduced in
 *  Duktape 2.2).  Check different cases in built-in init code.
 */

/*===
true
true
true
true
===*/

function test() {
    var f1 = Number;    // top level function
    var f2 = Math.cos;  // function-valued property
    var f3 = Object.getOwnPropertyDescriptor(RegExp.prototype, 'global').get;  // getter
    var f4 = Object.getOwnPropertyDescriptor(Object.prototype, '__proto__').set;  // setter

/*
    print(f1);
    print(f2);
    print(f3);
    print(f4);
*/

    print(Object.getPrototypeOf(f1) === Function.prototype);
    print(Object.getPrototypeOf(f2) === Function.prototype);
    print(Object.getPrototypeOf(f3) === Function.prototype);
    print(Object.getPrototypeOf(f4) === Function.prototype);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
