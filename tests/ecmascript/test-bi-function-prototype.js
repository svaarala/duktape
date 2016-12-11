/*
 *  Function prototype
 */

/*===
prototype
[object Function]
true
true
true 0
true true
undefined undefined
===*/

function functionPrototypeTest() {
    var ret;

    print(Object.prototype.toString.call(Function.prototype));
    print(Object.getPrototypeOf(Function.prototype) === Object.prototype);
    print(Object.isExtensible(Function.prototype));
    print('length' in Function.prototype, Function.prototype.length);
    print('constructor' in Function.prototype, Function.prototype.constructor === Function);

    ret = Function.prototype('foo', 'bar');
    print(typeof ret, ret);
}

try {
    print('prototype');
    functionPrototypeTest();
} catch (e) {
    print(e.stack || e);
}
