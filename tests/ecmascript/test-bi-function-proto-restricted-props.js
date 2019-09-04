/*
 *  ES2015+: CreateRealm() initializes Function.prototype with
 *  AddRestrictedFunctionProperties(), "caller" and "arguments".
 *  These properties are no longer in function instances.
 */

/*===
true
true
true
true
object
function
function
object
function
function
true
true
true
true
true
true
true
true
===*/

try {
    var f1 = function () {};
    var f2 = function () { 'use strict'; }
    var pd1, pd2;

    // In ES2015+ non-strict objects don't have these properties but
    // e.g. V8 provides them as nulls, probably for legacy reasons.
    pd1 = Object.getOwnPropertyDescriptor(f1, 'caller');
    print(pd1 === void 0);
    pd1 = Object.getOwnPropertyDescriptor(f1, 'arguments');
    print(pd1 === void 0);

    pd1 = Object.getOwnPropertyDescriptor(f2, 'caller');
    print(pd1 === void 0);
    pd1 = Object.getOwnPropertyDescriptor(f2, 'arguments');
    print(pd1 === void 0);

    pd1 = Object.getOwnPropertyDescriptor(Function.prototype, 'caller');
    print(typeof pd1);
    print(typeof pd1.get);
    print(typeof pd1.set);
    pd2 = Object.getOwnPropertyDescriptor(Function.prototype, 'arguments');
    print(typeof pd2);
    print(typeof pd2.get);
    print(typeof pd2.set);
    print(pd1.get !== void 0);
    print(pd1.set !== void 0);
    print(pd1.get === pd1.set);
    print(pd2.get !== void 0);
    print(pd2.set !== void 0);
    print(pd2.get === pd1.set);
    print(pd1.get === pd2.get);
    print(pd1.set === pd2.set);
} catch (e) {
    print(e.stack || e);
}
