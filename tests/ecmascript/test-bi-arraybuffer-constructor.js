/*
 *  ArrayBuffer constructor
 */

/*@include util-typedarray.js@*/

/*---
{
    "custom": true
}
---*/

function str(x) {
    if (typeof x === 'function') { return '[function]'; }
    return String(x);
}

/*===
string NONE object 0 0 0 1
undefined undefined object 0 0 0 1
object null object 0 0 0 1
boolean true object 1 1 0 1
boolean false object 0 0 0 1
string foo object 0 0 0 1
string 123 object 123 123 0 1
object 123 object 123 123 0 1
object [object Object] object 0 0 0 1
function [function] object 0 0 0 1
object [object Object] object 321 321 0 1
number -Infinity RangeError
number -1000000000 RangeError
number -1 RangeError
number 0 object 0 0 0 1
number 1 object 1 1 0 1
number 100 object 100 100 0 1
number 1000000 object 1000000 1000000 0 1
number Infinity RangeError
number NaN object 0 0 0 1
===*/

function arrayBufferConstructorTest() {
    /*
     *  TypedArray spec only provides: new ArrayBuffer(unsigned long length).
     *
     *  Duktape uses a ToInteger() coercion for the argument so it accepts a
     *  wider range of arguments.
     */

    [
        'NONE', undefined, null, true, false, 'foo', '123', [ '123' ],
        { foo: 'bar' }, function func() {},
        { valueOf: function () { return 321; } },
        -1 / 0, -1e9, -1, 0, 1, 100, 1e6, 1 / 0, 0 / 0
    ].forEach(function (arg) {
        var b;
        try {
            if (arg === 'NONE') {
                b = new ArrayBuffer();
            } else {
                b = new ArrayBuffer(arg);
            }
            print(typeof arg, str(arg), typeof b, b.length, b.byteLength, b.byteOffset, b.BYTES_PER_ELEMENT);

            if (typeof b.byteLength === 'number') {
                for (var i = 0; i < b.byteLength; i++) {
                    if (b[i] != 0) {
                        throw new Error('arraybuffer not zeroed as expected, non-zero byte at index: ' + i);
                    }
                }
            }
        } catch (e) {
            print(typeof arg, str(arg), e.name);
        }
    });
}

try {
    arrayBufferConstructorTest();
} catch (e) {
    print(e.stack || e);
}

/*===
TypeError
===*/

/* ArrayBuffer cannot be called as a normal function. */

function arrayBufferNormalCallTest() {
    var buf = ArrayBuffer(10);
    print(typeof buf);  // Never here
}

try {
    arrayBufferNormalCallTest();
} catch (e) {
    print(e.name);
}

/*===
ArrayBuffer constructor call test
[object ArrayBuffer]
[object ArrayBuffer]
true
[object ArrayBuffer] -> ArrayBuffer.prototype -> Object.prototype
true false false false
true
[object ArrayBuffer]
ArrayBuffer normal call test
TypeError
===*/

function arrayBufferConstructorCallTest(constructorCall) {
    var b;
    var pd;

    if (constructorCall) {
        b = new ArrayBuffer(16);
    } else {
        b = ArrayBuffer(16);
    }

    print(String(b));
    print(Object.prototype.toString.call(b));

    // The required internal prototype is the initial value of
    // ArrayBuffer.prototype (%ArrayBufferPrototype%), with
    // writable = enumerable = configurable = false.

    print(Object.getPrototypeOf(b) === ArrayBuffer.prototype);
    printPrototypeChain(b);

    pd = Object.getOwnPropertyDescriptor(ArrayBuffer, 'prototype') || {};
    print(pd.value === Object.getPrototypeOf(b), pd.writable, pd.enumerable, pd.configurable);

    // .toString() is inherited from Object.prototype
    print(b.toString === Object.prototype.toString);
    print(b.toString());
}

try {
    print('ArrayBuffer constructor call test');
    arrayBufferConstructorCallTest(true);
} catch (e) {
    print(e.stack || e);
}

try {
    print('ArrayBuffer normal call test');
    arrayBufferConstructorCallTest(false);
} catch (e) {
    print(e.name);
}

/*===
ArrayBuffer array-like test
0
===*/

/* There's no constructor variant for an Array-like initializer, although such
 * an initializer is accepted by e.g. V8.  Test for the compliant behavior, i.e.
 * reject such a call; constructor argument is ToNumber() + ToLength() coerced.
 */

function arrayBufferArrayLikeTest() {
    // Because ToNumber([1, 2, 3]) is NaN, it ToLength() coerces to 0, and the
    // result is a 0-length ArrayBuffer.

    var b = new ArrayBuffer([ 1, 2, 3 ]);

    print(b.byteLength);
}

try {
    print('ArrayBuffer array-like test');
    arrayBufferArrayLikeTest();
} catch (e) {
    print(e.stack || e);
}

/*===
ArrayBuffer argument test
NONE 0
undefined 0
null 0
true 1
false 0
3 3
3.9 3
-1 RangeError
-1.5 RangeError
0 0
0 0
NaN 0
268435456 268435456
1073741824 1073741824
4294967296 RangeError
[object Object] 17
===*/

/* Test the ArrayBuffer argument coercion.
 *
 * Currently Duktape doesn't follow ES6 ToLength() (which *clamps* to
 * [0,2^53-1]).
 */

function arrayBufferArgumentTest() {
    var b;
    var MB = 1024 * 1024;

    [
        'NONE',

        // Undefined ToNumber() coerces to NaN and fails the final
        // SameValueZero() comparison.

        undefined,

        // Null ToNumber() coerces to zero so it's accepted.
        null,

        // True ToNumber() coerces to 1 and false to 0, both accepted.

        true,
        false,

        // ES6 requires that ToNumber(arg) match ToLength(ToNumber(arg))
        // (using SameValueZero which ignores zero sign).  In other words,
        // fractional input values cause a RangeError.  V8 doesn't follow
        // this behavior but truncates instead.

        3,
        3.9,

        // Negative values are coerced to zero by ToLength().  But, later on
        // ToNumber(arg) and ToLength(ToNumber(arg)) are compared to ensure
        // that they match with SameValueZero.  If not, a RangeError should
        // happen.  So a negative value should RangeError.

        -1,
        -1.5,

        // Negative zero is explicitly allowed by ES6, the SameValueZero()
        // comparison ignores zero sign.

        -0,
        +0,

        // NaN is not allowed: it coerces to zero but won't pass the
        // SameValueZero() comparison.

        0 / 0,

        // Test that 256MB allocation works

        256 * MB,

        // Test that 1GB allocation works -- note that this fails on V8/Node.js.

        1024 * MB,

        // For now there's an internal maximum buffer length in Duktape.
        // It is triggered at least by 2^32 so test for that.  Exceeding
        // the limit causes a RangeError.  ES6 doesn't specify what to do
        // if an internal limit is reached but a RangeError seems suitable
        // and matches V8.

        4096 * MB,
        { valueOf: function () { return 17; } },
    ].forEach(function (arg) {
        try {
            if (arg === 'NONE') {
                b = new ArrayBuffer();
            } else {
                b = new ArrayBuffer(arg);  // ToNumber() floors -> 3
            }
            print(arg, b.byteLength);
        } catch (e) {
            print(arg, e.name);
        }
    });
}

try {
    print('ArrayBuffer argument test');
    arrayBufferArgumentTest();
} catch (e) {
    print(e.stack || e);
}
