/*
 *  instanceof and @@hasInstance
 */

/*===
- No @@hasInstance case; inherited from Function.prototype
false
true
- Function overrides @@hasInstance
false
true
- Function inherits a non-standard @@hasInstance
inherited hasInstance true 123
true
inherited hasInstance true [object Object]
true
- Function inherits a non-standard @@hasInstance, but function itself also provides an undefined @@hasInstance
false
true
true
- Same but overriding value is null
false
true
true
- Same but overriding value is not undefined/null, but also not a callable object
TypeError
- Same, plain non-callable object
TypeError
- @@hasInstance is a getter with side effects
@@hasInstance getter
hasInstance true 123
true
@@hasInstance getter
hasInstance true [object Object]
true
- Function.prototype[@@hasInstance] exists
true
true
false
function false false false
- Function.prototype[@@hasInstance] access to OrdinaryHasInstance()
false
true
false
true
===*/

function basicTest() {
    print('- No @@hasInstance case; inherited from Function.prototype');
    var rhs = function () {};
    print(123 instanceof rhs);
    print(Object.create(rhs.prototype) instanceof rhs);

    print('- Function overrides @@hasInstance');
    var rhs = function () {};
    rhs[Symbol.hasInstance] = function (v) {
        print('hasInstance', this === rhs, v);
        return 1;
    }
    print(123 instanceof rhs);
    print(Object.create(rhs.prototype) instanceof rhs);

    print('- Function inherits a non-standard @@hasInstance');
    var rhs = function () {};
    var o = {};
    Object.setPrototypeOf(rhs, o);
    o[Symbol.hasInstance] = function (v) {
        print('inherited hasInstance', this === rhs, v);
        return true;
    }
    print(123 instanceof rhs);
    print(Object.create(rhs.prototype) instanceof rhs);

    // In this case we fall back to OrdinaryHasInstance().
    print('- Function inherits a non-standard @@hasInstance, but function itself also provides an undefined @@hasInstance');
    var rhs = function () {};
    var o = {};
    Object.setPrototypeOf(rhs, o);
    o[Symbol.hasInstance] = function (v) {
        print('undefined hasInstance', this === rhs, v);
        return 1;
    }
    rhs[Symbol.hasInstance] = void 0;
    print(123 instanceof rhs);
    print(Object.create(rhs.prototype) instanceof rhs);
    var inst = new rhs();
    print(inst instanceof rhs);

    // For null value, GetMethod (https://www.ecma-international.org/ecma-262/6.0/#sec-getmethod)
    // returns 'undefined' for both undefined AND null, so that InstanceofOperator(O, C)
    // handles them the same in Step 4 of https://www.ecma-international.org/ecma-262/6.0/#sec-instanceofoperator.
    // In other words, for both undefined and null we must fall back to the
    // OrdinaryHasInstance() algorithm.  V8 treats null and undefined differently
    // (TypeError for null), Firefox treats them the same.

    // https://www.ecma-international.org/ecma-262/6.0/#sec-instanceofoperator
    // A non-undefined/null value, causes a "not callable" TypeError
    print('- Same but overriding value is null');
    var rhs = function () {};
    var o = {};
    Object.setPrototypeOf(rhs, o);
    o[Symbol.hasInstance] = function (v) {
        print('null hasInstance', this === rhs, v);
        return 1;
    }
    rhs[Symbol.hasInstance] = null;
    print(123 instanceof rhs);
    print(Object.create(rhs.prototype) instanceof rhs);
    var inst = new rhs();
    print(inst instanceof rhs);

    print('- Same but overriding value is not undefined/null, but also not a callable object');
    var rhs = function () {};
    var o = {};
    Object.setPrototypeOf(rhs, o);
    o[Symbol.hasInstance] = function (v) {
        print('fail hasInstance', this === rhs, v);
        return 1;
    }
    try {
        rhs[Symbol.hasInstance] = true;
        print(123 instanceof rhs);
    } catch (e) {
        print(e.name);
    }

    print('- Same, plain non-callable object');
    var rhs = function () {};
    var o = {};
    Object.setPrototypeOf(rhs, o);
    o[Symbol.hasInstance] = function (v) {
        print('fail hasInstance', this === rhs, v);
        return 1;
    }
    try {
        rhs[Symbol.hasInstance] = { plain: true };
        print(123 instanceof rhs);
    } catch (e) {
        print(e.name);
    }

    print('- @@hasInstance is a getter with side effects');
    var rhs = function () {};
    Object.defineProperty(rhs, Symbol.hasInstance, {
        get: function () {
            print('@@hasInstance getter');
            return function (v) {
                print('hasInstance', this === rhs, v);
                return 1;
            }
        }
    });
    print(123 instanceof rhs);
    print(Object.create(rhs.prototype) instanceof rhs);

    print('- Function.prototype[@@hasInstance] exists');
    print(Symbol.hasInstance in Function.prototype);
    print(Function.prototype[Symbol.hasInstance].call(Error, new RangeError()));
    print(Function.prototype[Symbol.hasInstance].call(Error, new Date()));
    var pd = Object.getOwnPropertyDescriptor(Function.prototype, Symbol.hasInstance);
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);

    // Function.prototype[@@hasInstance] allows direct access to the
    // ES2015 OrdinaryHasInstance() specification method.
    print('- Function.prototype[@@hasInstance] access to OrdinaryHasInstance()');
    var rhs = function () {};
    rhs[Symbol.hasInstance] = function (v) {
        print('hasInstance', this === rhs, v);
        return 0;
    }
    print(123 instanceof rhs);
    print(Object.create(rhs.prototype) instanceof rhs);
    print(Function.prototype[Symbol.hasInstance].call(rhs, {}));
    print(Function.prototype[Symbol.hasInstance].call(rhs, Object.create(rhs.prototype)));
}

try {
    basicTest();
} catch (e) {
    print(e.stack || e);
}
