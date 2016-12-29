/*
 *  Typed array constructors inherit from %TypedArray% in ES2015.
 */

/*===
Int8Array false true
Uint8Array false true
Uint8ClampedArray false true
Int16Array false true
Uint16Array false true
Int32Array false true
Uint32Array false true
Float32Array false true
Float64Array false true
TypeError
TypeError
object true false false false
string TypedArray false false true
number 0 false false true
===*/

function test() {
    [ Int8Array, Uint8Array, Uint8ClampedArray,
      Int16Array, Uint16Array, Int32Array, Uint32Array,
      Float32Array, Float64Array ].forEach(function (fn) {
        print(fn.name, Object.getPrototypeOf(fn) === Function.prototype, Object.getPrototypeOf(fn) === Object.getPrototypeOf(Int8Array));
    });

    // The %TypedArray% rejects normal and constructor calls.
    var TA = Object.getPrototypeOf(Int8Array);
    try {
        TA();
    } catch (e) {
        print(e.name);
    }
    try {
        new TA();
    } catch (e) {
        print(e.name);
    }

    // %TypedArray%.prototype is %TypedArrayPrototype%.
    var TAP = Object.getPrototypeOf(Object.getPrototypeOf(new Uint8Array(1)));
    pd = Object.getOwnPropertyDescriptor(TA, 'prototype');
    print(typeof pd.value, pd.value === TAP, pd.writable, pd.enumerable, pd.configurable);

    // .name
    pd = Object.getOwnPropertyDescriptor(TA, 'name');
    print(typeof pd.value, pd.value, pd.writable, pd.enumerable, pd.configurable);

    // .length
    pd = Object.getOwnPropertyDescriptor(TA, 'length');
    print(typeof pd.value, pd.value, pd.writable, pd.enumerable, pd.configurable);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
