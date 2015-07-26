/*
 *  DataView constructor
 */

/*@include util-typedarray.js@*/

/*---
{
    "custom": true
}
---*/

/*===
DataView constructor call test
[object DataView]
[object DataView]
object true
true
[object DataView] -> DataView.prototype -> Object.prototype
true false false false
true
[object DataView]
DataView normal call test
TypeError
===*/

function dataViewConstructorCallTest(constructorCall) {
    var b = new ArrayBuffer(16);
    var v;
    var pd;

    if (constructorCall) {
        v = new DataView(b, 4, 7);
    } else {
        v = DataView(b, 4, 7);
    }

    print(String(v));
    print(Object.prototype.toString.call(v));
    print(typeof v.buffer, v.buffer === b);

    // The required internal prototype is the initial value of
    // ArrayBuffer.prototype (%ArrayBufferPrototype%), with
    // writable = enumerable = configurable = false.

    print(Object.getPrototypeOf(v) === DataView.prototype);
    printPrototypeChain(v);

    pd = Object.getOwnPropertyDescriptor(DataView, 'prototype') || {};
    print(pd.value === Object.getPrototypeOf(v), pd.writable, pd.enumerable, pd.configurable);

    // .toString() is inherited from Object.prototype
    print(v.toString === Object.prototype.toString);
    print(v.toString());
}

try {
    print('DataView constructor call test');
    dataViewConstructorCallTest(true);
} catch (e) {
    print(e.stack || e);
}

try {
    print('DataView normal call test');
    dataViewConstructorCallTest(false);
} catch (e) {
    print(e.name);
}

/*===
DataView argument test
 TypeError
 TypeError
 TypeError
true TypeError
false TypeError
[object Object] TypeError
[object ArrayBuffer] true 0 16
[object ArrayBuffer], true 0 16
[object ArrayBuffer], true 0 16
[object ArrayBuffer],true true 1 15
[object ArrayBuffer],false true 0 16
[object ArrayBuffer],[object Object] true 6 10
[object ArrayBuffer],-3.9 RangeError
[object ArrayBuffer],-3 RangeError
[object ArrayBuffer],0 true 0 16
[object ArrayBuffer],0 true 0 16
[object ArrayBuffer],1 true 1 15
[object ArrayBuffer],3.9 true 3 13
[object ArrayBuffer],15 true 15 1
[object ArrayBuffer],16 true 16 0
[object ArrayBuffer],17 RangeError
[object ArrayBuffer],0,0 true 0 0
[object ArrayBuffer],0,1 true 0 1
[object ArrayBuffer],0,15 true 0 15
[object ArrayBuffer],0,16 true 0 16
[object ArrayBuffer],0,17 RangeError
[object ArrayBuffer],1, true 1 15
[object ArrayBuffer],1, true 1 0
[object ArrayBuffer],1,true true 1 1
[object ArrayBuffer],1,false true 1 0
[object ArrayBuffer],[object Object],[object Object] true 6 4
[object ArrayBuffer],1,0 true 1 0
[object ArrayBuffer],1,1 true 1 1
[object ArrayBuffer],1,14 true 1 14
[object ArrayBuffer],1,15 true 1 15
[object ArrayBuffer],1,16 RangeError
[object ArrayBuffer],15,-3 RangeError
[object ArrayBuffer],15,-1 RangeError
[object ArrayBuffer],15,0 true 15 0
[object ArrayBuffer],15,0 true 15 0
[object ArrayBuffer],15,1 true 15 1
[object ArrayBuffer],15,2 RangeError
[object ArrayBuffer],16,0 true 16 0
[object ArrayBuffer],16,1 RangeError
[object ArrayBuffer],17,0 RangeError
[object ArrayBuffer],17,1 RangeError
[object ArrayBuffer],13,2,dummy true 13 2
===*/

function dataViewArgumentTest() {
    var b = new ArrayBuffer(16);

    // new DataView(buffer, [byteOffset], [byteLength])

    [
        [],

        // buffer must be an object and must have an internal
        // [[ArrayBufferData]] slot (i.e. be an ArrayBuffer)

        [ undefined ],
        [ null ],
        [ true ],
        [ false ],
        [ {} ],
        [ b ],

        // Offset must satisfy:
        //     - ToNumber(offset) === ToInteger(ToNumber(offset))
        //     - integer offset must be >= 0
        //     - integer offset must be <= buffer byteLength

        [ b, undefined ],
        [ b, null ],
        [ b, true ],
        [ b, false ],
        [ b, { valueOf: function () { return 6; } } ],
        [ b, -3.9 ],
        [ b, -3 ],
        [ b, -0 ],
        [ b, +0 ],
        [ b, 1 ],
        [ b, 3.9 ],
        [ b, 15 ],
        [ b, 16 ],
        [ b, 17 ],

        // Length must satisfy:
        //     - if undefined, viewByteLength becomes #remaining bytes after offset
        //     - ToLength(byteLength) + byteOffset <= bufferByteLength

        [ b, +0, 0 ],
        [ b, +0, 1 ],
        [ b, +0, 15 ],
        [ b, +0, 16 ],
        [ b, +0, 17 ],

        [ b, 1, undefined ],
        [ b, 1, null ],
        [ b, 1, true ],
        [ b, 1, false ],
        [ b, { valueOf: function () { return 6; } }, { valueOf: function () { return 4; } } ],

        [ b, 1, 0 ],
        [ b, 1, 1 ],
        [ b, 1, 14 ],
        [ b, 1, 15 ],
        [ b, 1, 16 ],

        [ b, 15, -3 ],
        [ b, 15, -1 ],
        [ b, 15, -0 ],
        [ b, 15, +0 ],
        [ b, 15, 1 ],
        [ b, 15, 2 ],

        [ b, 16, 0 ],
        [ b, 16, 1 ],

        [ b, 17, 0 ],
        [ b, 17, 1 ],

        // Additional argument is ignored

        [ b, 13, 2, 'dummy' ]
    ].forEach(function (arglist) {
        try {
            if (arglist.length === 0) {
                v = new DataView();
            } else if (arglist.length === 1) {
                v = new DataView(arglist[0]);
            } else if (arglist.length === 2) {
                v = new DataView(arglist[0], arglist[1]);
            } else if (arglist.length === 3) {
                v = new DataView(arglist[0], arglist[1], arglist[2]);
            } else if (arglist.length === 4) {
                v = new DataView(arglist[0], arglist[1], arglist[2], arglist[3]);
            } else {
                throw new Error('internal error');
            }

            // DataView doesn't have a 'length' property or virtual indices.
            print(arglist, v.buffer === b, v.byteOffset, v.byteLength);
        } catch (e) {
            print(arglist, e.name);
        }
    });
}

try {
    print('DataView argument test');
    dataViewArgumentTest();
} catch (e) {
    print(e.stack || e);
}
