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
string "" false false true
===*/

function functionPrototypeTest() {
    var ret;
    var pd;

    print(Object.prototype.toString.call(Function.prototype));
    print(Object.getPrototypeOf(Function.prototype) === Object.prototype);
    print(Object.isExtensible(Function.prototype));
    print('length' in Function.prototype, Function.prototype.length);
    print('constructor' in Function.prototype, Function.prototype.constructor === Function);

    ret = Function.prototype('foo', 'bar');
    print(typeof ret, ret);

    // ES2015 added '.name'.  Function.prototype provides an empty default name.
    // It is not writable, but it is configurable so that Object.defineProperty()
    // can be used to set the name of an anonymous function.
    pd = Object.getOwnPropertyDescriptor(Function.prototype, 'name');
    print(typeof pd.value, JSON.stringify(pd.value), pd.writable, pd.enumerable, pd.configurable);
}

try {
    print('prototype');
    functionPrototypeTest();
} catch (e) {
    print(e.stack || e);
}
