/*
 *  TypedArray constructor
 */

/*@include util-typedarray.js@*/

/*---
{
    "custom": true
}
---*/

function printProps(x) {
    print(typeof x, Object.prototype.toString.call(x), x.length, x.byteLength, x.byteOffset);
}

/*===
TypedArray constructor call test
new Int8Array(b, 8, 1)
[object Int8Array]
[object Int8Array]
[object Int8Array] -> Int8Array.prototype -> TypedArray.prototype -> Object.prototype
true false false false
true
[object Int8Array]
new Uint8Array(b, 8, 1)
[object Uint8Array]
[object Uint8Array]
[object Uint8Array] -> Uint8Array.prototype -> TypedArray.prototype -> Object.prototype
true false false false
true
[object Uint8Array]
new Uint8ClampedArray(b, 8, 1)
[object Uint8ClampedArray]
[object Uint8ClampedArray]
[object Uint8ClampedArray] -> Uint8ClampedArray.prototype -> TypedArray.prototype -> Object.prototype
true false false false
true
[object Uint8ClampedArray]
new Int16Array(b, 8, 1)
[object Int16Array]
[object Int16Array]
[object Int16Array] -> Int16Array.prototype -> TypedArray.prototype -> Object.prototype
true false false false
true
[object Int16Array]
new Uint16Array(b, 8, 1)
[object Uint16Array]
[object Uint16Array]
[object Uint16Array] -> Uint16Array.prototype -> TypedArray.prototype -> Object.prototype
true false false false
true
[object Uint16Array]
new Int32Array(b, 8, 1)
[object Int32Array]
[object Int32Array]
[object Int32Array] -> Int32Array.prototype -> TypedArray.prototype -> Object.prototype
true false false false
true
[object Int32Array]
new Uint32Array(b, 8, 1)
[object Uint32Array]
[object Uint32Array]
[object Uint32Array] -> Uint32Array.prototype -> TypedArray.prototype -> Object.prototype
true false false false
true
[object Uint32Array]
new Float32Array(b, 8, 1)
[object Float32Array]
[object Float32Array]
[object Float32Array] -> Float32Array.prototype -> TypedArray.prototype -> Object.prototype
true false false false
true
[object Float32Array]
new Float64Array(b, 8, 1)
[object Float64Array]
[object Float64Array]
[object Float64Array] -> Float64Array.prototype -> TypedArray.prototype -> Object.prototype
true false false false
true
[object Float64Array]
TypedArray normal call test
Int8Array(b, 8, 1)
TypeError
Uint8Array(b, 8, 1)
TypeError
Uint8ClampedArray(b, 8, 1)
TypeError
Int16Array(b, 8, 1)
TypeError
Uint16Array(b, 8, 1)
TypeError
Int32Array(b, 8, 1)
TypeError
Uint32Array(b, 8, 1)
TypeError
Float32Array(b, 8, 1)
TypeError
Float64Array(b, 8, 1)
TypeError
===*/

function typedArrayConstructorCallTest(constructorCall) {
    var b = new ArrayBuffer(16);
    var v;
    var evalstr;
    var pd;

    [
        'Int8Array',
        'Uint8Array',
        'Uint8ClampedArray',
        'Int16Array',
        'Uint16Array',
        'Int32Array',
        'Uint32Array',
        'Float32Array',
        'Float64Array'
    ].forEach(function (consname) {
        if (constructorCall) {
            evalstr = 'new ';
        } else {
            evalstr = '';
        }
        evalstr += consname + '(b, 8, 1)';
        try {
            print(evalstr);
            v = eval(evalstr);

            print(String(v));
            print(Object.prototype.toString.call(v));

            printPrototypeChain(v);

            pd = Object.getOwnPropertyDescriptor(eval(consname), 'prototype') || {};
            print(pd.value === Object.getPrototypeOf(v), pd.writable, pd.enumerable, pd.configurable);

            // .toString() is inherited from Object.prototype
            print(v.toString === Object.prototype.toString);
            print(v.toString());
        } catch (e) {
            print(e.name);
        }
    });
}

try {
    print('TypedArray constructor call test');
    typedArrayConstructorCallTest(true);
} catch (e) {
    print(e.stack || e);
}

try {
    print('TypedArray normal call test');
    typedArrayConstructorCallTest(false);
} catch (e) {
    print(e.stack || e);
}

/*===
TypedArray argument bruteforce test
 new Int8Array() false 0 0 0
 new Int8Array(arglist[0]) false 0 0 0
 new Int8Array(arglist[0]) false 0 0 0
true new Int8Array(arglist[0]) false 1 0 1
false new Int8Array(arglist[0]) false 0 0 0
[object Object] new Int8Array(arglist[0]) false 0 0 0
[object ArrayBuffer] new Int8Array(arglist[0]) true 16 0 16
[object ArrayBuffer], new Int8Array(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer], new Int8Array(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer],true new Int8Array(arglist[0], arglist[1]) true 15 1 15
[object ArrayBuffer],false new Int8Array(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer],[object Object] new Int8Array(arglist[0], arglist[1]) true 10 6 10
[object ArrayBuffer],[object Object] new Int8Array(arglist[0], arglist[1]) true 8 8 8
[object ArrayBuffer],-3.9 RangeError
[object ArrayBuffer],-3 RangeError
[object ArrayBuffer],0 new Int8Array(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer],0 new Int8Array(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer],1 new Int8Array(arglist[0], arglist[1]) true 15 1 15
[object ArrayBuffer],2 new Int8Array(arglist[0], arglist[1]) true 14 2 14
[object ArrayBuffer],3 new Int8Array(arglist[0], arglist[1]) true 13 3 13
[object ArrayBuffer],4 new Int8Array(arglist[0], arglist[1]) true 12 4 12
[object ArrayBuffer],5 new Int8Array(arglist[0], arglist[1]) true 11 5 11
[object ArrayBuffer],6 new Int8Array(arglist[0], arglist[1]) true 10 6 10
[object ArrayBuffer],7 new Int8Array(arglist[0], arglist[1]) true 9 7 9
[object ArrayBuffer],8 new Int8Array(arglist[0], arglist[1]) true 8 8 8
[object ArrayBuffer],3.9 new Int8Array(arglist[0], arglist[1]) true 13 3 13
[object ArrayBuffer],15 new Int8Array(arglist[0], arglist[1]) true 1 15 1
[object ArrayBuffer],16 new Int8Array(arglist[0], arglist[1]) true 0 16 0
[object ArrayBuffer],17 RangeError
[object ArrayBuffer],0,0 new Int8Array(arglist[0], arglist[1], arglist[2]) true 0 0 0
[object ArrayBuffer],0,1 new Int8Array(arglist[0], arglist[1], arglist[2]) true 1 0 1
[object ArrayBuffer],0,2 new Int8Array(arglist[0], arglist[1], arglist[2]) true 2 0 2
[object ArrayBuffer],0,3 new Int8Array(arglist[0], arglist[1], arglist[2]) true 3 0 3
[object ArrayBuffer],0,4 new Int8Array(arglist[0], arglist[1], arglist[2]) true 4 0 4
[object ArrayBuffer],0,5 new Int8Array(arglist[0], arglist[1], arglist[2]) true 5 0 5
[object ArrayBuffer],0,6 new Int8Array(arglist[0], arglist[1], arglist[2]) true 6 0 6
[object ArrayBuffer],0,7 new Int8Array(arglist[0], arglist[1], arglist[2]) true 7 0 7
[object ArrayBuffer],0,8 new Int8Array(arglist[0], arglist[1], arglist[2]) true 8 0 8
[object ArrayBuffer],0,9 new Int8Array(arglist[0], arglist[1], arglist[2]) true 9 0 9
[object ArrayBuffer],0,10 new Int8Array(arglist[0], arglist[1], arglist[2]) true 10 0 10
[object ArrayBuffer],0,11 new Int8Array(arglist[0], arglist[1], arglist[2]) true 11 0 11
[object ArrayBuffer],0,12 new Int8Array(arglist[0], arglist[1], arglist[2]) true 12 0 12
[object ArrayBuffer],0,13 new Int8Array(arglist[0], arglist[1], arglist[2]) true 13 0 13
[object ArrayBuffer],0,14 new Int8Array(arglist[0], arglist[1], arglist[2]) true 14 0 14
[object ArrayBuffer],0,15 new Int8Array(arglist[0], arglist[1], arglist[2]) true 15 0 15
[object ArrayBuffer],0,16 new Int8Array(arglist[0], arglist[1], arglist[2]) true 16 0 16
[object ArrayBuffer],0,17 RangeError
[object ArrayBuffer],1, new Int8Array(arglist[0], arglist[1], arglist[2]) true 15 1 15
[object ArrayBuffer],1, new Int8Array(arglist[0], arglist[1], arglist[2]) true 0 1 0
[object ArrayBuffer],1,true new Int8Array(arglist[0], arglist[1], arglist[2]) true 1 1 1
[object ArrayBuffer],1,false new Int8Array(arglist[0], arglist[1], arglist[2]) true 0 1 0
[object ArrayBuffer],[object Object],[object Object] new Int8Array(arglist[0], arglist[1], arglist[2]) true 4 6 4
[object ArrayBuffer],1,0 new Int8Array(arglist[0], arglist[1], arglist[2]) true 0 1 0
[object ArrayBuffer],1,1 new Int8Array(arglist[0], arglist[1], arglist[2]) true 1 1 1
[object ArrayBuffer],1,14 new Int8Array(arglist[0], arglist[1], arglist[2]) true 14 1 14
[object ArrayBuffer],1,15 new Int8Array(arglist[0], arglist[1], arglist[2]) true 15 1 15
[object ArrayBuffer],1,16 RangeError
[object ArrayBuffer],8, new Int8Array(arglist[0], arglist[1], arglist[2]) true 8 8 8
[object ArrayBuffer],8, new Int8Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,true new Int8Array(arglist[0], arglist[1], arglist[2]) true 1 8 1
[object ArrayBuffer],8,false new Int8Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],[object Object],[object Object] new Int8Array(arglist[0], arglist[1], arglist[2]) true 2 8 2
[object ArrayBuffer],4,0 new Int8Array(arglist[0], arglist[1], arglist[2]) true 0 4 0
[object ArrayBuffer],4,1 new Int8Array(arglist[0], arglist[1], arglist[2]) true 1 4 1
[object ArrayBuffer],4,2 new Int8Array(arglist[0], arglist[1], arglist[2]) true 2 4 2
[object ArrayBuffer],4,3 new Int8Array(arglist[0], arglist[1], arglist[2]) true 3 4 3
[object ArrayBuffer],4,4 new Int8Array(arglist[0], arglist[1], arglist[2]) true 4 4 4
[object ArrayBuffer],4,5 new Int8Array(arglist[0], arglist[1], arglist[2]) true 5 4 5
[object ArrayBuffer],4,6 new Int8Array(arglist[0], arglist[1], arglist[2]) true 6 4 6
[object ArrayBuffer],4,7 new Int8Array(arglist[0], arglist[1], arglist[2]) true 7 4 7
[object ArrayBuffer],4,8 new Int8Array(arglist[0], arglist[1], arglist[2]) true 8 4 8
[object ArrayBuffer],4,9 new Int8Array(arglist[0], arglist[1], arglist[2]) true 9 4 9
[object ArrayBuffer],4,10 new Int8Array(arglist[0], arglist[1], arglist[2]) true 10 4 10
[object ArrayBuffer],4,11 new Int8Array(arglist[0], arglist[1], arglist[2]) true 11 4 11
[object ArrayBuffer],4,12 new Int8Array(arglist[0], arglist[1], arglist[2]) true 12 4 12
[object ArrayBuffer],4,13 RangeError
[object ArrayBuffer],4,14 RangeError
[object ArrayBuffer],4,15 RangeError
[object ArrayBuffer],4,16 RangeError
[object ArrayBuffer],8,0 new Int8Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,1 new Int8Array(arglist[0], arglist[1], arglist[2]) true 1 8 1
[object ArrayBuffer],8,2 new Int8Array(arglist[0], arglist[1], arglist[2]) true 2 8 2
[object ArrayBuffer],8,3 new Int8Array(arglist[0], arglist[1], arglist[2]) true 3 8 3
[object ArrayBuffer],8,4 new Int8Array(arglist[0], arglist[1], arglist[2]) true 4 8 4
[object ArrayBuffer],8,5 new Int8Array(arglist[0], arglist[1], arglist[2]) true 5 8 5
[object ArrayBuffer],8,6 new Int8Array(arglist[0], arglist[1], arglist[2]) true 6 8 6
[object ArrayBuffer],8,7 new Int8Array(arglist[0], arglist[1], arglist[2]) true 7 8 7
[object ArrayBuffer],8,8 new Int8Array(arglist[0], arglist[1], arglist[2]) true 8 8 8
[object ArrayBuffer],8,9 RangeError
[object ArrayBuffer],8,10 RangeError
[object ArrayBuffer],8,11 RangeError
[object ArrayBuffer],8,12 RangeError
[object ArrayBuffer],8,13 RangeError
[object ArrayBuffer],8,14 RangeError
[object ArrayBuffer],8,15 RangeError
[object ArrayBuffer],8,16 RangeError
[object ArrayBuffer],15,-3 RangeError
[object ArrayBuffer],15,-1 RangeError
[object ArrayBuffer],15,0 new Int8Array(arglist[0], arglist[1], arglist[2]) true 0 15 0
[object ArrayBuffer],15,0 new Int8Array(arglist[0], arglist[1], arglist[2]) true 0 15 0
[object ArrayBuffer],15,1 new Int8Array(arglist[0], arglist[1], arglist[2]) true 1 15 1
[object ArrayBuffer],15,2 RangeError
[object ArrayBuffer],16,0 new Int8Array(arglist[0], arglist[1], arglist[2]) true 0 16 0
[object ArrayBuffer],16,1 RangeError
[object ArrayBuffer],17,0 RangeError
[object ArrayBuffer],17,1 RangeError
[object ArrayBuffer],8,1,dummy new Int8Array(arglist[0], arglist[1], arglist[2], arglist[3]) true 1 8 1
 new Uint8Array() false 0 0 0
 new Uint8Array(arglist[0]) false 0 0 0
 new Uint8Array(arglist[0]) false 0 0 0
true new Uint8Array(arglist[0]) false 1 0 1
false new Uint8Array(arglist[0]) false 0 0 0
[object Object] new Uint8Array(arglist[0]) false 0 0 0
[object ArrayBuffer] new Uint8Array(arglist[0]) true 16 0 16
[object ArrayBuffer], new Uint8Array(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer], new Uint8Array(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer],true new Uint8Array(arglist[0], arglist[1]) true 15 1 15
[object ArrayBuffer],false new Uint8Array(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer],[object Object] new Uint8Array(arglist[0], arglist[1]) true 10 6 10
[object ArrayBuffer],[object Object] new Uint8Array(arglist[0], arglist[1]) true 8 8 8
[object ArrayBuffer],-3.9 RangeError
[object ArrayBuffer],-3 RangeError
[object ArrayBuffer],0 new Uint8Array(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer],0 new Uint8Array(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer],1 new Uint8Array(arglist[0], arglist[1]) true 15 1 15
[object ArrayBuffer],2 new Uint8Array(arglist[0], arglist[1]) true 14 2 14
[object ArrayBuffer],3 new Uint8Array(arglist[0], arglist[1]) true 13 3 13
[object ArrayBuffer],4 new Uint8Array(arglist[0], arglist[1]) true 12 4 12
[object ArrayBuffer],5 new Uint8Array(arglist[0], arglist[1]) true 11 5 11
[object ArrayBuffer],6 new Uint8Array(arglist[0], arglist[1]) true 10 6 10
[object ArrayBuffer],7 new Uint8Array(arglist[0], arglist[1]) true 9 7 9
[object ArrayBuffer],8 new Uint8Array(arglist[0], arglist[1]) true 8 8 8
[object ArrayBuffer],3.9 new Uint8Array(arglist[0], arglist[1]) true 13 3 13
[object ArrayBuffer],15 new Uint8Array(arglist[0], arglist[1]) true 1 15 1
[object ArrayBuffer],16 new Uint8Array(arglist[0], arglist[1]) true 0 16 0
[object ArrayBuffer],17 RangeError
[object ArrayBuffer],0,0 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 0 0 0
[object ArrayBuffer],0,1 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 1 0 1
[object ArrayBuffer],0,2 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 2 0 2
[object ArrayBuffer],0,3 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 3 0 3
[object ArrayBuffer],0,4 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 4 0 4
[object ArrayBuffer],0,5 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 5 0 5
[object ArrayBuffer],0,6 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 6 0 6
[object ArrayBuffer],0,7 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 7 0 7
[object ArrayBuffer],0,8 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 8 0 8
[object ArrayBuffer],0,9 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 9 0 9
[object ArrayBuffer],0,10 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 10 0 10
[object ArrayBuffer],0,11 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 11 0 11
[object ArrayBuffer],0,12 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 12 0 12
[object ArrayBuffer],0,13 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 13 0 13
[object ArrayBuffer],0,14 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 14 0 14
[object ArrayBuffer],0,15 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 15 0 15
[object ArrayBuffer],0,16 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 16 0 16
[object ArrayBuffer],0,17 RangeError
[object ArrayBuffer],1, new Uint8Array(arglist[0], arglist[1], arglist[2]) true 15 1 15
[object ArrayBuffer],1, new Uint8Array(arglist[0], arglist[1], arglist[2]) true 0 1 0
[object ArrayBuffer],1,true new Uint8Array(arglist[0], arglist[1], arglist[2]) true 1 1 1
[object ArrayBuffer],1,false new Uint8Array(arglist[0], arglist[1], arglist[2]) true 0 1 0
[object ArrayBuffer],[object Object],[object Object] new Uint8Array(arglist[0], arglist[1], arglist[2]) true 4 6 4
[object ArrayBuffer],1,0 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 0 1 0
[object ArrayBuffer],1,1 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 1 1 1
[object ArrayBuffer],1,14 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 14 1 14
[object ArrayBuffer],1,15 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 15 1 15
[object ArrayBuffer],1,16 RangeError
[object ArrayBuffer],8, new Uint8Array(arglist[0], arglist[1], arglist[2]) true 8 8 8
[object ArrayBuffer],8, new Uint8Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,true new Uint8Array(arglist[0], arglist[1], arglist[2]) true 1 8 1
[object ArrayBuffer],8,false new Uint8Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],[object Object],[object Object] new Uint8Array(arglist[0], arglist[1], arglist[2]) true 2 8 2
[object ArrayBuffer],4,0 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 0 4 0
[object ArrayBuffer],4,1 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 1 4 1
[object ArrayBuffer],4,2 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 2 4 2
[object ArrayBuffer],4,3 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 3 4 3
[object ArrayBuffer],4,4 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 4 4 4
[object ArrayBuffer],4,5 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 5 4 5
[object ArrayBuffer],4,6 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 6 4 6
[object ArrayBuffer],4,7 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 7 4 7
[object ArrayBuffer],4,8 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 8 4 8
[object ArrayBuffer],4,9 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 9 4 9
[object ArrayBuffer],4,10 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 10 4 10
[object ArrayBuffer],4,11 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 11 4 11
[object ArrayBuffer],4,12 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 12 4 12
[object ArrayBuffer],4,13 RangeError
[object ArrayBuffer],4,14 RangeError
[object ArrayBuffer],4,15 RangeError
[object ArrayBuffer],4,16 RangeError
[object ArrayBuffer],8,0 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,1 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 1 8 1
[object ArrayBuffer],8,2 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 2 8 2
[object ArrayBuffer],8,3 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 3 8 3
[object ArrayBuffer],8,4 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 4 8 4
[object ArrayBuffer],8,5 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 5 8 5
[object ArrayBuffer],8,6 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 6 8 6
[object ArrayBuffer],8,7 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 7 8 7
[object ArrayBuffer],8,8 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 8 8 8
[object ArrayBuffer],8,9 RangeError
[object ArrayBuffer],8,10 RangeError
[object ArrayBuffer],8,11 RangeError
[object ArrayBuffer],8,12 RangeError
[object ArrayBuffer],8,13 RangeError
[object ArrayBuffer],8,14 RangeError
[object ArrayBuffer],8,15 RangeError
[object ArrayBuffer],8,16 RangeError
[object ArrayBuffer],15,-3 RangeError
[object ArrayBuffer],15,-1 RangeError
[object ArrayBuffer],15,0 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 0 15 0
[object ArrayBuffer],15,0 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 0 15 0
[object ArrayBuffer],15,1 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 1 15 1
[object ArrayBuffer],15,2 RangeError
[object ArrayBuffer],16,0 new Uint8Array(arglist[0], arglist[1], arglist[2]) true 0 16 0
[object ArrayBuffer],16,1 RangeError
[object ArrayBuffer],17,0 RangeError
[object ArrayBuffer],17,1 RangeError
[object ArrayBuffer],8,1,dummy new Uint8Array(arglist[0], arglist[1], arglist[2], arglist[3]) true 1 8 1
 new Uint8ClampedArray() false 0 0 0
 new Uint8ClampedArray(arglist[0]) false 0 0 0
 new Uint8ClampedArray(arglist[0]) false 0 0 0
true new Uint8ClampedArray(arglist[0]) false 1 0 1
false new Uint8ClampedArray(arglist[0]) false 0 0 0
[object Object] new Uint8ClampedArray(arglist[0]) false 0 0 0
[object ArrayBuffer] new Uint8ClampedArray(arglist[0]) true 16 0 16
[object ArrayBuffer], new Uint8ClampedArray(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer], new Uint8ClampedArray(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer],true new Uint8ClampedArray(arglist[0], arglist[1]) true 15 1 15
[object ArrayBuffer],false new Uint8ClampedArray(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer],[object Object] new Uint8ClampedArray(arglist[0], arglist[1]) true 10 6 10
[object ArrayBuffer],[object Object] new Uint8ClampedArray(arglist[0], arglist[1]) true 8 8 8
[object ArrayBuffer],-3.9 RangeError
[object ArrayBuffer],-3 RangeError
[object ArrayBuffer],0 new Uint8ClampedArray(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer],0 new Uint8ClampedArray(arglist[0], arglist[1]) true 16 0 16
[object ArrayBuffer],1 new Uint8ClampedArray(arglist[0], arglist[1]) true 15 1 15
[object ArrayBuffer],2 new Uint8ClampedArray(arglist[0], arglist[1]) true 14 2 14
[object ArrayBuffer],3 new Uint8ClampedArray(arglist[0], arglist[1]) true 13 3 13
[object ArrayBuffer],4 new Uint8ClampedArray(arglist[0], arglist[1]) true 12 4 12
[object ArrayBuffer],5 new Uint8ClampedArray(arglist[0], arglist[1]) true 11 5 11
[object ArrayBuffer],6 new Uint8ClampedArray(arglist[0], arglist[1]) true 10 6 10
[object ArrayBuffer],7 new Uint8ClampedArray(arglist[0], arglist[1]) true 9 7 9
[object ArrayBuffer],8 new Uint8ClampedArray(arglist[0], arglist[1]) true 8 8 8
[object ArrayBuffer],3.9 new Uint8ClampedArray(arglist[0], arglist[1]) true 13 3 13
[object ArrayBuffer],15 new Uint8ClampedArray(arglist[0], arglist[1]) true 1 15 1
[object ArrayBuffer],16 new Uint8ClampedArray(arglist[0], arglist[1]) true 0 16 0
[object ArrayBuffer],17 RangeError
[object ArrayBuffer],0,0 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 0 0 0
[object ArrayBuffer],0,1 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 1 0 1
[object ArrayBuffer],0,2 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 2 0 2
[object ArrayBuffer],0,3 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 3 0 3
[object ArrayBuffer],0,4 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 4 0 4
[object ArrayBuffer],0,5 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 5 0 5
[object ArrayBuffer],0,6 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 6 0 6
[object ArrayBuffer],0,7 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 7 0 7
[object ArrayBuffer],0,8 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 8 0 8
[object ArrayBuffer],0,9 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 9 0 9
[object ArrayBuffer],0,10 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 10 0 10
[object ArrayBuffer],0,11 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 11 0 11
[object ArrayBuffer],0,12 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 12 0 12
[object ArrayBuffer],0,13 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 13 0 13
[object ArrayBuffer],0,14 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 14 0 14
[object ArrayBuffer],0,15 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 15 0 15
[object ArrayBuffer],0,16 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 16 0 16
[object ArrayBuffer],0,17 RangeError
[object ArrayBuffer],1, new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 15 1 15
[object ArrayBuffer],1, new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 0 1 0
[object ArrayBuffer],1,true new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 1 1 1
[object ArrayBuffer],1,false new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 0 1 0
[object ArrayBuffer],[object Object],[object Object] new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 4 6 4
[object ArrayBuffer],1,0 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 0 1 0
[object ArrayBuffer],1,1 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 1 1 1
[object ArrayBuffer],1,14 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 14 1 14
[object ArrayBuffer],1,15 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 15 1 15
[object ArrayBuffer],1,16 RangeError
[object ArrayBuffer],8, new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 8 8 8
[object ArrayBuffer],8, new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,true new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 1 8 1
[object ArrayBuffer],8,false new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],[object Object],[object Object] new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 2 8 2
[object ArrayBuffer],4,0 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 0 4 0
[object ArrayBuffer],4,1 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 1 4 1
[object ArrayBuffer],4,2 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 2 4 2
[object ArrayBuffer],4,3 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 3 4 3
[object ArrayBuffer],4,4 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 4 4 4
[object ArrayBuffer],4,5 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 5 4 5
[object ArrayBuffer],4,6 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 6 4 6
[object ArrayBuffer],4,7 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 7 4 7
[object ArrayBuffer],4,8 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 8 4 8
[object ArrayBuffer],4,9 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 9 4 9
[object ArrayBuffer],4,10 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 10 4 10
[object ArrayBuffer],4,11 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 11 4 11
[object ArrayBuffer],4,12 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 12 4 12
[object ArrayBuffer],4,13 RangeError
[object ArrayBuffer],4,14 RangeError
[object ArrayBuffer],4,15 RangeError
[object ArrayBuffer],4,16 RangeError
[object ArrayBuffer],8,0 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,1 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 1 8 1
[object ArrayBuffer],8,2 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 2 8 2
[object ArrayBuffer],8,3 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 3 8 3
[object ArrayBuffer],8,4 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 4 8 4
[object ArrayBuffer],8,5 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 5 8 5
[object ArrayBuffer],8,6 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 6 8 6
[object ArrayBuffer],8,7 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 7 8 7
[object ArrayBuffer],8,8 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 8 8 8
[object ArrayBuffer],8,9 RangeError
[object ArrayBuffer],8,10 RangeError
[object ArrayBuffer],8,11 RangeError
[object ArrayBuffer],8,12 RangeError
[object ArrayBuffer],8,13 RangeError
[object ArrayBuffer],8,14 RangeError
[object ArrayBuffer],8,15 RangeError
[object ArrayBuffer],8,16 RangeError
[object ArrayBuffer],15,-3 RangeError
[object ArrayBuffer],15,-1 RangeError
[object ArrayBuffer],15,0 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 0 15 0
[object ArrayBuffer],15,0 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 0 15 0
[object ArrayBuffer],15,1 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 1 15 1
[object ArrayBuffer],15,2 RangeError
[object ArrayBuffer],16,0 new Uint8ClampedArray(arglist[0], arglist[1], arglist[2]) true 0 16 0
[object ArrayBuffer],16,1 RangeError
[object ArrayBuffer],17,0 RangeError
[object ArrayBuffer],17,1 RangeError
[object ArrayBuffer],8,1,dummy new Uint8ClampedArray(arglist[0], arglist[1], arglist[2], arglist[3]) true 1 8 1
 new Int16Array() false 0 0 0
 new Int16Array(arglist[0]) false 0 0 0
 new Int16Array(arglist[0]) false 0 0 0
true new Int16Array(arglist[0]) false 1 0 2
false new Int16Array(arglist[0]) false 0 0 0
[object Object] new Int16Array(arglist[0]) false 0 0 0
[object ArrayBuffer] new Int16Array(arglist[0]) true 8 0 16
[object ArrayBuffer], new Int16Array(arglist[0], arglist[1]) true 8 0 16
[object ArrayBuffer], new Int16Array(arglist[0], arglist[1]) true 8 0 16
[object ArrayBuffer],true RangeError
[object ArrayBuffer],false new Int16Array(arglist[0], arglist[1]) true 8 0 16
[object ArrayBuffer],[object Object] new Int16Array(arglist[0], arglist[1]) true 5 6 10
[object ArrayBuffer],[object Object] new Int16Array(arglist[0], arglist[1]) true 4 8 8
[object ArrayBuffer],-3.9 RangeError
[object ArrayBuffer],-3 RangeError
[object ArrayBuffer],0 new Int16Array(arglist[0], arglist[1]) true 8 0 16
[object ArrayBuffer],0 new Int16Array(arglist[0], arglist[1]) true 8 0 16
[object ArrayBuffer],1 RangeError
[object ArrayBuffer],2 new Int16Array(arglist[0], arglist[1]) true 7 2 14
[object ArrayBuffer],3 RangeError
[object ArrayBuffer],4 new Int16Array(arglist[0], arglist[1]) true 6 4 12
[object ArrayBuffer],5 RangeError
[object ArrayBuffer],6 new Int16Array(arglist[0], arglist[1]) true 5 6 10
[object ArrayBuffer],7 RangeError
[object ArrayBuffer],8 new Int16Array(arglist[0], arglist[1]) true 4 8 8
[object ArrayBuffer],3.9 RangeError
[object ArrayBuffer],15 RangeError
[object ArrayBuffer],16 new Int16Array(arglist[0], arglist[1]) true 0 16 0
[object ArrayBuffer],17 RangeError
[object ArrayBuffer],0,0 new Int16Array(arglist[0], arglist[1], arglist[2]) true 0 0 0
[object ArrayBuffer],0,1 new Int16Array(arglist[0], arglist[1], arglist[2]) true 1 0 2
[object ArrayBuffer],0,2 new Int16Array(arglist[0], arglist[1], arglist[2]) true 2 0 4
[object ArrayBuffer],0,3 new Int16Array(arglist[0], arglist[1], arglist[2]) true 3 0 6
[object ArrayBuffer],0,4 new Int16Array(arglist[0], arglist[1], arglist[2]) true 4 0 8
[object ArrayBuffer],0,5 new Int16Array(arglist[0], arglist[1], arglist[2]) true 5 0 10
[object ArrayBuffer],0,6 new Int16Array(arglist[0], arglist[1], arglist[2]) true 6 0 12
[object ArrayBuffer],0,7 new Int16Array(arglist[0], arglist[1], arglist[2]) true 7 0 14
[object ArrayBuffer],0,8 new Int16Array(arglist[0], arglist[1], arglist[2]) true 8 0 16
[object ArrayBuffer],0,9 RangeError
[object ArrayBuffer],0,10 RangeError
[object ArrayBuffer],0,11 RangeError
[object ArrayBuffer],0,12 RangeError
[object ArrayBuffer],0,13 RangeError
[object ArrayBuffer],0,14 RangeError
[object ArrayBuffer],0,15 RangeError
[object ArrayBuffer],0,16 RangeError
[object ArrayBuffer],0,17 RangeError
[object ArrayBuffer],1, RangeError
[object ArrayBuffer],1, RangeError
[object ArrayBuffer],1,true RangeError
[object ArrayBuffer],1,false RangeError
[object ArrayBuffer],[object Object],[object Object] new Int16Array(arglist[0], arglist[1], arglist[2]) true 4 6 8
[object ArrayBuffer],1,0 RangeError
[object ArrayBuffer],1,1 RangeError
[object ArrayBuffer],1,14 RangeError
[object ArrayBuffer],1,15 RangeError
[object ArrayBuffer],1,16 RangeError
[object ArrayBuffer],8, new Int16Array(arglist[0], arglist[1], arglist[2]) true 4 8 8
[object ArrayBuffer],8, new Int16Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,true new Int16Array(arglist[0], arglist[1], arglist[2]) true 1 8 2
[object ArrayBuffer],8,false new Int16Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],[object Object],[object Object] new Int16Array(arglist[0], arglist[1], arglist[2]) true 2 8 4
[object ArrayBuffer],4,0 new Int16Array(arglist[0], arglist[1], arglist[2]) true 0 4 0
[object ArrayBuffer],4,1 new Int16Array(arglist[0], arglist[1], arglist[2]) true 1 4 2
[object ArrayBuffer],4,2 new Int16Array(arglist[0], arglist[1], arglist[2]) true 2 4 4
[object ArrayBuffer],4,3 new Int16Array(arglist[0], arglist[1], arglist[2]) true 3 4 6
[object ArrayBuffer],4,4 new Int16Array(arglist[0], arglist[1], arglist[2]) true 4 4 8
[object ArrayBuffer],4,5 new Int16Array(arglist[0], arglist[1], arglist[2]) true 5 4 10
[object ArrayBuffer],4,6 new Int16Array(arglist[0], arglist[1], arglist[2]) true 6 4 12
[object ArrayBuffer],4,7 RangeError
[object ArrayBuffer],4,8 RangeError
[object ArrayBuffer],4,9 RangeError
[object ArrayBuffer],4,10 RangeError
[object ArrayBuffer],4,11 RangeError
[object ArrayBuffer],4,12 RangeError
[object ArrayBuffer],4,13 RangeError
[object ArrayBuffer],4,14 RangeError
[object ArrayBuffer],4,15 RangeError
[object ArrayBuffer],4,16 RangeError
[object ArrayBuffer],8,0 new Int16Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,1 new Int16Array(arglist[0], arglist[1], arglist[2]) true 1 8 2
[object ArrayBuffer],8,2 new Int16Array(arglist[0], arglist[1], arglist[2]) true 2 8 4
[object ArrayBuffer],8,3 new Int16Array(arglist[0], arglist[1], arglist[2]) true 3 8 6
[object ArrayBuffer],8,4 new Int16Array(arglist[0], arglist[1], arglist[2]) true 4 8 8
[object ArrayBuffer],8,5 RangeError
[object ArrayBuffer],8,6 RangeError
[object ArrayBuffer],8,7 RangeError
[object ArrayBuffer],8,8 RangeError
[object ArrayBuffer],8,9 RangeError
[object ArrayBuffer],8,10 RangeError
[object ArrayBuffer],8,11 RangeError
[object ArrayBuffer],8,12 RangeError
[object ArrayBuffer],8,13 RangeError
[object ArrayBuffer],8,14 RangeError
[object ArrayBuffer],8,15 RangeError
[object ArrayBuffer],8,16 RangeError
[object ArrayBuffer],15,-3 RangeError
[object ArrayBuffer],15,-1 RangeError
[object ArrayBuffer],15,0 RangeError
[object ArrayBuffer],15,0 RangeError
[object ArrayBuffer],15,1 RangeError
[object ArrayBuffer],15,2 RangeError
[object ArrayBuffer],16,0 new Int16Array(arglist[0], arglist[1], arglist[2]) true 0 16 0
[object ArrayBuffer],16,1 RangeError
[object ArrayBuffer],17,0 RangeError
[object ArrayBuffer],17,1 RangeError
[object ArrayBuffer],8,1,dummy new Int16Array(arglist[0], arglist[1], arglist[2], arglist[3]) true 1 8 2
 new Uint16Array() false 0 0 0
 new Uint16Array(arglist[0]) false 0 0 0
 new Uint16Array(arglist[0]) false 0 0 0
true new Uint16Array(arglist[0]) false 1 0 2
false new Uint16Array(arglist[0]) false 0 0 0
[object Object] new Uint16Array(arglist[0]) false 0 0 0
[object ArrayBuffer] new Uint16Array(arglist[0]) true 8 0 16
[object ArrayBuffer], new Uint16Array(arglist[0], arglist[1]) true 8 0 16
[object ArrayBuffer], new Uint16Array(arglist[0], arglist[1]) true 8 0 16
[object ArrayBuffer],true RangeError
[object ArrayBuffer],false new Uint16Array(arglist[0], arglist[1]) true 8 0 16
[object ArrayBuffer],[object Object] new Uint16Array(arglist[0], arglist[1]) true 5 6 10
[object ArrayBuffer],[object Object] new Uint16Array(arglist[0], arglist[1]) true 4 8 8
[object ArrayBuffer],-3.9 RangeError
[object ArrayBuffer],-3 RangeError
[object ArrayBuffer],0 new Uint16Array(arglist[0], arglist[1]) true 8 0 16
[object ArrayBuffer],0 new Uint16Array(arglist[0], arglist[1]) true 8 0 16
[object ArrayBuffer],1 RangeError
[object ArrayBuffer],2 new Uint16Array(arglist[0], arglist[1]) true 7 2 14
[object ArrayBuffer],3 RangeError
[object ArrayBuffer],4 new Uint16Array(arglist[0], arglist[1]) true 6 4 12
[object ArrayBuffer],5 RangeError
[object ArrayBuffer],6 new Uint16Array(arglist[0], arglist[1]) true 5 6 10
[object ArrayBuffer],7 RangeError
[object ArrayBuffer],8 new Uint16Array(arglist[0], arglist[1]) true 4 8 8
[object ArrayBuffer],3.9 RangeError
[object ArrayBuffer],15 RangeError
[object ArrayBuffer],16 new Uint16Array(arglist[0], arglist[1]) true 0 16 0
[object ArrayBuffer],17 RangeError
[object ArrayBuffer],0,0 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 0 0 0
[object ArrayBuffer],0,1 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 1 0 2
[object ArrayBuffer],0,2 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 2 0 4
[object ArrayBuffer],0,3 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 3 0 6
[object ArrayBuffer],0,4 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 4 0 8
[object ArrayBuffer],0,5 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 5 0 10
[object ArrayBuffer],0,6 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 6 0 12
[object ArrayBuffer],0,7 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 7 0 14
[object ArrayBuffer],0,8 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 8 0 16
[object ArrayBuffer],0,9 RangeError
[object ArrayBuffer],0,10 RangeError
[object ArrayBuffer],0,11 RangeError
[object ArrayBuffer],0,12 RangeError
[object ArrayBuffer],0,13 RangeError
[object ArrayBuffer],0,14 RangeError
[object ArrayBuffer],0,15 RangeError
[object ArrayBuffer],0,16 RangeError
[object ArrayBuffer],0,17 RangeError
[object ArrayBuffer],1, RangeError
[object ArrayBuffer],1, RangeError
[object ArrayBuffer],1,true RangeError
[object ArrayBuffer],1,false RangeError
[object ArrayBuffer],[object Object],[object Object] new Uint16Array(arglist[0], arglist[1], arglist[2]) true 4 6 8
[object ArrayBuffer],1,0 RangeError
[object ArrayBuffer],1,1 RangeError
[object ArrayBuffer],1,14 RangeError
[object ArrayBuffer],1,15 RangeError
[object ArrayBuffer],1,16 RangeError
[object ArrayBuffer],8, new Uint16Array(arglist[0], arglist[1], arglist[2]) true 4 8 8
[object ArrayBuffer],8, new Uint16Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,true new Uint16Array(arglist[0], arglist[1], arglist[2]) true 1 8 2
[object ArrayBuffer],8,false new Uint16Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],[object Object],[object Object] new Uint16Array(arglist[0], arglist[1], arglist[2]) true 2 8 4
[object ArrayBuffer],4,0 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 0 4 0
[object ArrayBuffer],4,1 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 1 4 2
[object ArrayBuffer],4,2 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 2 4 4
[object ArrayBuffer],4,3 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 3 4 6
[object ArrayBuffer],4,4 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 4 4 8
[object ArrayBuffer],4,5 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 5 4 10
[object ArrayBuffer],4,6 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 6 4 12
[object ArrayBuffer],4,7 RangeError
[object ArrayBuffer],4,8 RangeError
[object ArrayBuffer],4,9 RangeError
[object ArrayBuffer],4,10 RangeError
[object ArrayBuffer],4,11 RangeError
[object ArrayBuffer],4,12 RangeError
[object ArrayBuffer],4,13 RangeError
[object ArrayBuffer],4,14 RangeError
[object ArrayBuffer],4,15 RangeError
[object ArrayBuffer],4,16 RangeError
[object ArrayBuffer],8,0 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,1 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 1 8 2
[object ArrayBuffer],8,2 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 2 8 4
[object ArrayBuffer],8,3 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 3 8 6
[object ArrayBuffer],8,4 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 4 8 8
[object ArrayBuffer],8,5 RangeError
[object ArrayBuffer],8,6 RangeError
[object ArrayBuffer],8,7 RangeError
[object ArrayBuffer],8,8 RangeError
[object ArrayBuffer],8,9 RangeError
[object ArrayBuffer],8,10 RangeError
[object ArrayBuffer],8,11 RangeError
[object ArrayBuffer],8,12 RangeError
[object ArrayBuffer],8,13 RangeError
[object ArrayBuffer],8,14 RangeError
[object ArrayBuffer],8,15 RangeError
[object ArrayBuffer],8,16 RangeError
[object ArrayBuffer],15,-3 RangeError
[object ArrayBuffer],15,-1 RangeError
[object ArrayBuffer],15,0 RangeError
[object ArrayBuffer],15,0 RangeError
[object ArrayBuffer],15,1 RangeError
[object ArrayBuffer],15,2 RangeError
[object ArrayBuffer],16,0 new Uint16Array(arglist[0], arglist[1], arglist[2]) true 0 16 0
[object ArrayBuffer],16,1 RangeError
[object ArrayBuffer],17,0 RangeError
[object ArrayBuffer],17,1 RangeError
[object ArrayBuffer],8,1,dummy new Uint16Array(arglist[0], arglist[1], arglist[2], arglist[3]) true 1 8 2
 new Int32Array() false 0 0 0
 new Int32Array(arglist[0]) false 0 0 0
 new Int32Array(arglist[0]) false 0 0 0
true new Int32Array(arglist[0]) false 1 0 4
false new Int32Array(arglist[0]) false 0 0 0
[object Object] new Int32Array(arglist[0]) false 0 0 0
[object ArrayBuffer] new Int32Array(arglist[0]) true 4 0 16
[object ArrayBuffer], new Int32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer], new Int32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer],true RangeError
[object ArrayBuffer],false new Int32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer],[object Object] RangeError
[object ArrayBuffer],[object Object] new Int32Array(arglist[0], arglist[1]) true 2 8 8
[object ArrayBuffer],-3.9 RangeError
[object ArrayBuffer],-3 RangeError
[object ArrayBuffer],0 new Int32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer],0 new Int32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer],1 RangeError
[object ArrayBuffer],2 RangeError
[object ArrayBuffer],3 RangeError
[object ArrayBuffer],4 new Int32Array(arglist[0], arglist[1]) true 3 4 12
[object ArrayBuffer],5 RangeError
[object ArrayBuffer],6 RangeError
[object ArrayBuffer],7 RangeError
[object ArrayBuffer],8 new Int32Array(arglist[0], arglist[1]) true 2 8 8
[object ArrayBuffer],3.9 RangeError
[object ArrayBuffer],15 RangeError
[object ArrayBuffer],16 new Int32Array(arglist[0], arglist[1]) true 0 16 0
[object ArrayBuffer],17 RangeError
[object ArrayBuffer],0,0 new Int32Array(arglist[0], arglist[1], arglist[2]) true 0 0 0
[object ArrayBuffer],0,1 new Int32Array(arglist[0], arglist[1], arglist[2]) true 1 0 4
[object ArrayBuffer],0,2 new Int32Array(arglist[0], arglist[1], arglist[2]) true 2 0 8
[object ArrayBuffer],0,3 new Int32Array(arglist[0], arglist[1], arglist[2]) true 3 0 12
[object ArrayBuffer],0,4 new Int32Array(arglist[0], arglist[1], arglist[2]) true 4 0 16
[object ArrayBuffer],0,5 RangeError
[object ArrayBuffer],0,6 RangeError
[object ArrayBuffer],0,7 RangeError
[object ArrayBuffer],0,8 RangeError
[object ArrayBuffer],0,9 RangeError
[object ArrayBuffer],0,10 RangeError
[object ArrayBuffer],0,11 RangeError
[object ArrayBuffer],0,12 RangeError
[object ArrayBuffer],0,13 RangeError
[object ArrayBuffer],0,14 RangeError
[object ArrayBuffer],0,15 RangeError
[object ArrayBuffer],0,16 RangeError
[object ArrayBuffer],0,17 RangeError
[object ArrayBuffer],1, RangeError
[object ArrayBuffer],1, RangeError
[object ArrayBuffer],1,true RangeError
[object ArrayBuffer],1,false RangeError
[object ArrayBuffer],[object Object],[object Object] RangeError
[object ArrayBuffer],1,0 RangeError
[object ArrayBuffer],1,1 RangeError
[object ArrayBuffer],1,14 RangeError
[object ArrayBuffer],1,15 RangeError
[object ArrayBuffer],1,16 RangeError
[object ArrayBuffer],8, new Int32Array(arglist[0], arglist[1], arglist[2]) true 2 8 8
[object ArrayBuffer],8, new Int32Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,true new Int32Array(arglist[0], arglist[1], arglist[2]) true 1 8 4
[object ArrayBuffer],8,false new Int32Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],[object Object],[object Object] new Int32Array(arglist[0], arglist[1], arglist[2]) true 2 8 8
[object ArrayBuffer],4,0 new Int32Array(arglist[0], arglist[1], arglist[2]) true 0 4 0
[object ArrayBuffer],4,1 new Int32Array(arglist[0], arglist[1], arglist[2]) true 1 4 4
[object ArrayBuffer],4,2 new Int32Array(arglist[0], arglist[1], arglist[2]) true 2 4 8
[object ArrayBuffer],4,3 new Int32Array(arglist[0], arglist[1], arglist[2]) true 3 4 12
[object ArrayBuffer],4,4 RangeError
[object ArrayBuffer],4,5 RangeError
[object ArrayBuffer],4,6 RangeError
[object ArrayBuffer],4,7 RangeError
[object ArrayBuffer],4,8 RangeError
[object ArrayBuffer],4,9 RangeError
[object ArrayBuffer],4,10 RangeError
[object ArrayBuffer],4,11 RangeError
[object ArrayBuffer],4,12 RangeError
[object ArrayBuffer],4,13 RangeError
[object ArrayBuffer],4,14 RangeError
[object ArrayBuffer],4,15 RangeError
[object ArrayBuffer],4,16 RangeError
[object ArrayBuffer],8,0 new Int32Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,1 new Int32Array(arglist[0], arglist[1], arglist[2]) true 1 8 4
[object ArrayBuffer],8,2 new Int32Array(arglist[0], arglist[1], arglist[2]) true 2 8 8
[object ArrayBuffer],8,3 RangeError
[object ArrayBuffer],8,4 RangeError
[object ArrayBuffer],8,5 RangeError
[object ArrayBuffer],8,6 RangeError
[object ArrayBuffer],8,7 RangeError
[object ArrayBuffer],8,8 RangeError
[object ArrayBuffer],8,9 RangeError
[object ArrayBuffer],8,10 RangeError
[object ArrayBuffer],8,11 RangeError
[object ArrayBuffer],8,12 RangeError
[object ArrayBuffer],8,13 RangeError
[object ArrayBuffer],8,14 RangeError
[object ArrayBuffer],8,15 RangeError
[object ArrayBuffer],8,16 RangeError
[object ArrayBuffer],15,-3 RangeError
[object ArrayBuffer],15,-1 RangeError
[object ArrayBuffer],15,0 RangeError
[object ArrayBuffer],15,0 RangeError
[object ArrayBuffer],15,1 RangeError
[object ArrayBuffer],15,2 RangeError
[object ArrayBuffer],16,0 new Int32Array(arglist[0], arglist[1], arglist[2]) true 0 16 0
[object ArrayBuffer],16,1 RangeError
[object ArrayBuffer],17,0 RangeError
[object ArrayBuffer],17,1 RangeError
[object ArrayBuffer],8,1,dummy new Int32Array(arglist[0], arglist[1], arglist[2], arglist[3]) true 1 8 4
 new Uint32Array() false 0 0 0
 new Uint32Array(arglist[0]) false 0 0 0
 new Uint32Array(arglist[0]) false 0 0 0
true new Uint32Array(arglist[0]) false 1 0 4
false new Uint32Array(arglist[0]) false 0 0 0
[object Object] new Uint32Array(arglist[0]) false 0 0 0
[object ArrayBuffer] new Uint32Array(arglist[0]) true 4 0 16
[object ArrayBuffer], new Uint32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer], new Uint32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer],true RangeError
[object ArrayBuffer],false new Uint32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer],[object Object] RangeError
[object ArrayBuffer],[object Object] new Uint32Array(arglist[0], arglist[1]) true 2 8 8
[object ArrayBuffer],-3.9 RangeError
[object ArrayBuffer],-3 RangeError
[object ArrayBuffer],0 new Uint32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer],0 new Uint32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer],1 RangeError
[object ArrayBuffer],2 RangeError
[object ArrayBuffer],3 RangeError
[object ArrayBuffer],4 new Uint32Array(arglist[0], arglist[1]) true 3 4 12
[object ArrayBuffer],5 RangeError
[object ArrayBuffer],6 RangeError
[object ArrayBuffer],7 RangeError
[object ArrayBuffer],8 new Uint32Array(arglist[0], arglist[1]) true 2 8 8
[object ArrayBuffer],3.9 RangeError
[object ArrayBuffer],15 RangeError
[object ArrayBuffer],16 new Uint32Array(arglist[0], arglist[1]) true 0 16 0
[object ArrayBuffer],17 RangeError
[object ArrayBuffer],0,0 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 0 0 0
[object ArrayBuffer],0,1 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 1 0 4
[object ArrayBuffer],0,2 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 2 0 8
[object ArrayBuffer],0,3 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 3 0 12
[object ArrayBuffer],0,4 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 4 0 16
[object ArrayBuffer],0,5 RangeError
[object ArrayBuffer],0,6 RangeError
[object ArrayBuffer],0,7 RangeError
[object ArrayBuffer],0,8 RangeError
[object ArrayBuffer],0,9 RangeError
[object ArrayBuffer],0,10 RangeError
[object ArrayBuffer],0,11 RangeError
[object ArrayBuffer],0,12 RangeError
[object ArrayBuffer],0,13 RangeError
[object ArrayBuffer],0,14 RangeError
[object ArrayBuffer],0,15 RangeError
[object ArrayBuffer],0,16 RangeError
[object ArrayBuffer],0,17 RangeError
[object ArrayBuffer],1, RangeError
[object ArrayBuffer],1, RangeError
[object ArrayBuffer],1,true RangeError
[object ArrayBuffer],1,false RangeError
[object ArrayBuffer],[object Object],[object Object] RangeError
[object ArrayBuffer],1,0 RangeError
[object ArrayBuffer],1,1 RangeError
[object ArrayBuffer],1,14 RangeError
[object ArrayBuffer],1,15 RangeError
[object ArrayBuffer],1,16 RangeError
[object ArrayBuffer],8, new Uint32Array(arglist[0], arglist[1], arglist[2]) true 2 8 8
[object ArrayBuffer],8, new Uint32Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,true new Uint32Array(arglist[0], arglist[1], arglist[2]) true 1 8 4
[object ArrayBuffer],8,false new Uint32Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],[object Object],[object Object] new Uint32Array(arglist[0], arglist[1], arglist[2]) true 2 8 8
[object ArrayBuffer],4,0 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 0 4 0
[object ArrayBuffer],4,1 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 1 4 4
[object ArrayBuffer],4,2 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 2 4 8
[object ArrayBuffer],4,3 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 3 4 12
[object ArrayBuffer],4,4 RangeError
[object ArrayBuffer],4,5 RangeError
[object ArrayBuffer],4,6 RangeError
[object ArrayBuffer],4,7 RangeError
[object ArrayBuffer],4,8 RangeError
[object ArrayBuffer],4,9 RangeError
[object ArrayBuffer],4,10 RangeError
[object ArrayBuffer],4,11 RangeError
[object ArrayBuffer],4,12 RangeError
[object ArrayBuffer],4,13 RangeError
[object ArrayBuffer],4,14 RangeError
[object ArrayBuffer],4,15 RangeError
[object ArrayBuffer],4,16 RangeError
[object ArrayBuffer],8,0 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,1 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 1 8 4
[object ArrayBuffer],8,2 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 2 8 8
[object ArrayBuffer],8,3 RangeError
[object ArrayBuffer],8,4 RangeError
[object ArrayBuffer],8,5 RangeError
[object ArrayBuffer],8,6 RangeError
[object ArrayBuffer],8,7 RangeError
[object ArrayBuffer],8,8 RangeError
[object ArrayBuffer],8,9 RangeError
[object ArrayBuffer],8,10 RangeError
[object ArrayBuffer],8,11 RangeError
[object ArrayBuffer],8,12 RangeError
[object ArrayBuffer],8,13 RangeError
[object ArrayBuffer],8,14 RangeError
[object ArrayBuffer],8,15 RangeError
[object ArrayBuffer],8,16 RangeError
[object ArrayBuffer],15,-3 RangeError
[object ArrayBuffer],15,-1 RangeError
[object ArrayBuffer],15,0 RangeError
[object ArrayBuffer],15,0 RangeError
[object ArrayBuffer],15,1 RangeError
[object ArrayBuffer],15,2 RangeError
[object ArrayBuffer],16,0 new Uint32Array(arglist[0], arglist[1], arglist[2]) true 0 16 0
[object ArrayBuffer],16,1 RangeError
[object ArrayBuffer],17,0 RangeError
[object ArrayBuffer],17,1 RangeError
[object ArrayBuffer],8,1,dummy new Uint32Array(arglist[0], arglist[1], arglist[2], arglist[3]) true 1 8 4
 new Float32Array() false 0 0 0
 new Float32Array(arglist[0]) false 0 0 0
 new Float32Array(arglist[0]) false 0 0 0
true new Float32Array(arglist[0]) false 1 0 4
false new Float32Array(arglist[0]) false 0 0 0
[object Object] new Float32Array(arglist[0]) false 0 0 0
[object ArrayBuffer] new Float32Array(arglist[0]) true 4 0 16
[object ArrayBuffer], new Float32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer], new Float32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer],true RangeError
[object ArrayBuffer],false new Float32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer],[object Object] RangeError
[object ArrayBuffer],[object Object] new Float32Array(arglist[0], arglist[1]) true 2 8 8
[object ArrayBuffer],-3.9 RangeError
[object ArrayBuffer],-3 RangeError
[object ArrayBuffer],0 new Float32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer],0 new Float32Array(arglist[0], arglist[1]) true 4 0 16
[object ArrayBuffer],1 RangeError
[object ArrayBuffer],2 RangeError
[object ArrayBuffer],3 RangeError
[object ArrayBuffer],4 new Float32Array(arglist[0], arglist[1]) true 3 4 12
[object ArrayBuffer],5 RangeError
[object ArrayBuffer],6 RangeError
[object ArrayBuffer],7 RangeError
[object ArrayBuffer],8 new Float32Array(arglist[0], arglist[1]) true 2 8 8
[object ArrayBuffer],3.9 RangeError
[object ArrayBuffer],15 RangeError
[object ArrayBuffer],16 new Float32Array(arglist[0], arglist[1]) true 0 16 0
[object ArrayBuffer],17 RangeError
[object ArrayBuffer],0,0 new Float32Array(arglist[0], arglist[1], arglist[2]) true 0 0 0
[object ArrayBuffer],0,1 new Float32Array(arglist[0], arglist[1], arglist[2]) true 1 0 4
[object ArrayBuffer],0,2 new Float32Array(arglist[0], arglist[1], arglist[2]) true 2 0 8
[object ArrayBuffer],0,3 new Float32Array(arglist[0], arglist[1], arglist[2]) true 3 0 12
[object ArrayBuffer],0,4 new Float32Array(arglist[0], arglist[1], arglist[2]) true 4 0 16
[object ArrayBuffer],0,5 RangeError
[object ArrayBuffer],0,6 RangeError
[object ArrayBuffer],0,7 RangeError
[object ArrayBuffer],0,8 RangeError
[object ArrayBuffer],0,9 RangeError
[object ArrayBuffer],0,10 RangeError
[object ArrayBuffer],0,11 RangeError
[object ArrayBuffer],0,12 RangeError
[object ArrayBuffer],0,13 RangeError
[object ArrayBuffer],0,14 RangeError
[object ArrayBuffer],0,15 RangeError
[object ArrayBuffer],0,16 RangeError
[object ArrayBuffer],0,17 RangeError
[object ArrayBuffer],1, RangeError
[object ArrayBuffer],1, RangeError
[object ArrayBuffer],1,true RangeError
[object ArrayBuffer],1,false RangeError
[object ArrayBuffer],[object Object],[object Object] RangeError
[object ArrayBuffer],1,0 RangeError
[object ArrayBuffer],1,1 RangeError
[object ArrayBuffer],1,14 RangeError
[object ArrayBuffer],1,15 RangeError
[object ArrayBuffer],1,16 RangeError
[object ArrayBuffer],8, new Float32Array(arglist[0], arglist[1], arglist[2]) true 2 8 8
[object ArrayBuffer],8, new Float32Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,true new Float32Array(arglist[0], arglist[1], arglist[2]) true 1 8 4
[object ArrayBuffer],8,false new Float32Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],[object Object],[object Object] new Float32Array(arglist[0], arglist[1], arglist[2]) true 2 8 8
[object ArrayBuffer],4,0 new Float32Array(arglist[0], arglist[1], arglist[2]) true 0 4 0
[object ArrayBuffer],4,1 new Float32Array(arglist[0], arglist[1], arglist[2]) true 1 4 4
[object ArrayBuffer],4,2 new Float32Array(arglist[0], arglist[1], arglist[2]) true 2 4 8
[object ArrayBuffer],4,3 new Float32Array(arglist[0], arglist[1], arglist[2]) true 3 4 12
[object ArrayBuffer],4,4 RangeError
[object ArrayBuffer],4,5 RangeError
[object ArrayBuffer],4,6 RangeError
[object ArrayBuffer],4,7 RangeError
[object ArrayBuffer],4,8 RangeError
[object ArrayBuffer],4,9 RangeError
[object ArrayBuffer],4,10 RangeError
[object ArrayBuffer],4,11 RangeError
[object ArrayBuffer],4,12 RangeError
[object ArrayBuffer],4,13 RangeError
[object ArrayBuffer],4,14 RangeError
[object ArrayBuffer],4,15 RangeError
[object ArrayBuffer],4,16 RangeError
[object ArrayBuffer],8,0 new Float32Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,1 new Float32Array(arglist[0], arglist[1], arglist[2]) true 1 8 4
[object ArrayBuffer],8,2 new Float32Array(arglist[0], arglist[1], arglist[2]) true 2 8 8
[object ArrayBuffer],8,3 RangeError
[object ArrayBuffer],8,4 RangeError
[object ArrayBuffer],8,5 RangeError
[object ArrayBuffer],8,6 RangeError
[object ArrayBuffer],8,7 RangeError
[object ArrayBuffer],8,8 RangeError
[object ArrayBuffer],8,9 RangeError
[object ArrayBuffer],8,10 RangeError
[object ArrayBuffer],8,11 RangeError
[object ArrayBuffer],8,12 RangeError
[object ArrayBuffer],8,13 RangeError
[object ArrayBuffer],8,14 RangeError
[object ArrayBuffer],8,15 RangeError
[object ArrayBuffer],8,16 RangeError
[object ArrayBuffer],15,-3 RangeError
[object ArrayBuffer],15,-1 RangeError
[object ArrayBuffer],15,0 RangeError
[object ArrayBuffer],15,0 RangeError
[object ArrayBuffer],15,1 RangeError
[object ArrayBuffer],15,2 RangeError
[object ArrayBuffer],16,0 new Float32Array(arglist[0], arglist[1], arglist[2]) true 0 16 0
[object ArrayBuffer],16,1 RangeError
[object ArrayBuffer],17,0 RangeError
[object ArrayBuffer],17,1 RangeError
[object ArrayBuffer],8,1,dummy new Float32Array(arglist[0], arglist[1], arglist[2], arglist[3]) true 1 8 4
 new Float64Array() false 0 0 0
 new Float64Array(arglist[0]) false 0 0 0
 new Float64Array(arglist[0]) false 0 0 0
true new Float64Array(arglist[0]) false 1 0 8
false new Float64Array(arglist[0]) false 0 0 0
[object Object] new Float64Array(arglist[0]) false 0 0 0
[object ArrayBuffer] new Float64Array(arglist[0]) true 2 0 16
[object ArrayBuffer], new Float64Array(arglist[0], arglist[1]) true 2 0 16
[object ArrayBuffer], new Float64Array(arglist[0], arglist[1]) true 2 0 16
[object ArrayBuffer],true RangeError
[object ArrayBuffer],false new Float64Array(arglist[0], arglist[1]) true 2 0 16
[object ArrayBuffer],[object Object] RangeError
[object ArrayBuffer],[object Object] new Float64Array(arglist[0], arglist[1]) true 1 8 8
[object ArrayBuffer],-3.9 RangeError
[object ArrayBuffer],-3 RangeError
[object ArrayBuffer],0 new Float64Array(arglist[0], arglist[1]) true 2 0 16
[object ArrayBuffer],0 new Float64Array(arglist[0], arglist[1]) true 2 0 16
[object ArrayBuffer],1 RangeError
[object ArrayBuffer],2 RangeError
[object ArrayBuffer],3 RangeError
[object ArrayBuffer],4 RangeError
[object ArrayBuffer],5 RangeError
[object ArrayBuffer],6 RangeError
[object ArrayBuffer],7 RangeError
[object ArrayBuffer],8 new Float64Array(arglist[0], arglist[1]) true 1 8 8
[object ArrayBuffer],3.9 RangeError
[object ArrayBuffer],15 RangeError
[object ArrayBuffer],16 new Float64Array(arglist[0], arglist[1]) true 0 16 0
[object ArrayBuffer],17 RangeError
[object ArrayBuffer],0,0 new Float64Array(arglist[0], arglist[1], arglist[2]) true 0 0 0
[object ArrayBuffer],0,1 new Float64Array(arglist[0], arglist[1], arglist[2]) true 1 0 8
[object ArrayBuffer],0,2 new Float64Array(arglist[0], arglist[1], arglist[2]) true 2 0 16
[object ArrayBuffer],0,3 RangeError
[object ArrayBuffer],0,4 RangeError
[object ArrayBuffer],0,5 RangeError
[object ArrayBuffer],0,6 RangeError
[object ArrayBuffer],0,7 RangeError
[object ArrayBuffer],0,8 RangeError
[object ArrayBuffer],0,9 RangeError
[object ArrayBuffer],0,10 RangeError
[object ArrayBuffer],0,11 RangeError
[object ArrayBuffer],0,12 RangeError
[object ArrayBuffer],0,13 RangeError
[object ArrayBuffer],0,14 RangeError
[object ArrayBuffer],0,15 RangeError
[object ArrayBuffer],0,16 RangeError
[object ArrayBuffer],0,17 RangeError
[object ArrayBuffer],1, RangeError
[object ArrayBuffer],1, RangeError
[object ArrayBuffer],1,true RangeError
[object ArrayBuffer],1,false RangeError
[object ArrayBuffer],[object Object],[object Object] RangeError
[object ArrayBuffer],1,0 RangeError
[object ArrayBuffer],1,1 RangeError
[object ArrayBuffer],1,14 RangeError
[object ArrayBuffer],1,15 RangeError
[object ArrayBuffer],1,16 RangeError
[object ArrayBuffer],8, new Float64Array(arglist[0], arglist[1], arglist[2]) true 1 8 8
[object ArrayBuffer],8, new Float64Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,true new Float64Array(arglist[0], arglist[1], arglist[2]) true 1 8 8
[object ArrayBuffer],8,false new Float64Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],[object Object],[object Object] RangeError
[object ArrayBuffer],4,0 RangeError
[object ArrayBuffer],4,1 RangeError
[object ArrayBuffer],4,2 RangeError
[object ArrayBuffer],4,3 RangeError
[object ArrayBuffer],4,4 RangeError
[object ArrayBuffer],4,5 RangeError
[object ArrayBuffer],4,6 RangeError
[object ArrayBuffer],4,7 RangeError
[object ArrayBuffer],4,8 RangeError
[object ArrayBuffer],4,9 RangeError
[object ArrayBuffer],4,10 RangeError
[object ArrayBuffer],4,11 RangeError
[object ArrayBuffer],4,12 RangeError
[object ArrayBuffer],4,13 RangeError
[object ArrayBuffer],4,14 RangeError
[object ArrayBuffer],4,15 RangeError
[object ArrayBuffer],4,16 RangeError
[object ArrayBuffer],8,0 new Float64Array(arglist[0], arglist[1], arglist[2]) true 0 8 0
[object ArrayBuffer],8,1 new Float64Array(arglist[0], arglist[1], arglist[2]) true 1 8 8
[object ArrayBuffer],8,2 RangeError
[object ArrayBuffer],8,3 RangeError
[object ArrayBuffer],8,4 RangeError
[object ArrayBuffer],8,5 RangeError
[object ArrayBuffer],8,6 RangeError
[object ArrayBuffer],8,7 RangeError
[object ArrayBuffer],8,8 RangeError
[object ArrayBuffer],8,9 RangeError
[object ArrayBuffer],8,10 RangeError
[object ArrayBuffer],8,11 RangeError
[object ArrayBuffer],8,12 RangeError
[object ArrayBuffer],8,13 RangeError
[object ArrayBuffer],8,14 RangeError
[object ArrayBuffer],8,15 RangeError
[object ArrayBuffer],8,16 RangeError
[object ArrayBuffer],15,-3 RangeError
[object ArrayBuffer],15,-1 RangeError
[object ArrayBuffer],15,0 RangeError
[object ArrayBuffer],15,0 RangeError
[object ArrayBuffer],15,1 RangeError
[object ArrayBuffer],15,2 RangeError
[object ArrayBuffer],16,0 new Float64Array(arglist[0], arglist[1], arglist[2]) true 0 16 0
[object ArrayBuffer],16,1 RangeError
[object ArrayBuffer],17,0 RangeError
[object ArrayBuffer],17,1 RangeError
[object ArrayBuffer],8,1,dummy new Float64Array(arglist[0], arglist[1], arglist[2], arglist[3]) true 1 8 8
===*/

function typedArrayArgumentBruteForceTest() {
    var b = new ArrayBuffer(16);
    var v;
    var evalstr;
    var i;

    // new TypedArray(buffer, [byteOffset], [length])

    [
        'Int8Array',
        'Uint8Array',
        'Uint8ClampedArray',
        'Int16Array',
        'Uint16Array',
        'Int32Array',
        'Uint32Array',
        'Float32Array',
        'Float64Array'
    ].forEach(function (consname) {
        // The arguments for TypedArray views are different from DataView:
        //     - byteOffset must be a multiple of element byte size
        //     - length refers to length in elements, not bytes
        //       (the test values must provide combinations for 1, 2, 4, 8
        //       element byte sizes)
        //     - the "slice" must end inside the buffer; in particular the
        //       last element can't be partial

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
            //     - integer offset must be a multiple of element byte size
            //     - integer offset must be <= buffer byteLength
            //       (this check is implicit in the later steps)

            [ b, undefined ],
            [ b, null ],
            [ b, true ],
            [ b, false ],
            [ b, { valueOf: function () { return 6; } } ],
            [ b, { valueOf: function () { return 8; } } ],
            [ b, -3.9 ],
            [ b, -3 ],
            [ b, -0 ],
            [ b, +0 ],
            [ b, 1 ],
            [ b, 2 ],
            [ b, 3 ],
            [ b, 4 ],
            [ b, 5 ],
            [ b, 6 ],
            [ b, 7 ],
            [ b, 8 ],
            [ b, 3.9 ],
            [ b, 15 ],
            [ b, 16 ],
            [ b, 17 ],

            // Length must satisfy:
            //     - if undefined, remaining bytes must divide evenly with
            //       element byte size, length becomes viewByteLength / elementSize
            //     - ToLength(length) * elementSize + byteOffset <= bufferByteLength

            [ b, +0, 0 ],
            [ b, +0, 1 ],
            [ b, +0, 2 ],
            [ b, +0, 3 ],
            [ b, +0, 4 ],
            [ b, +0, 5 ],
            [ b, +0, 6 ],
            [ b, +0, 7 ],
            [ b, +0, 8 ],
            [ b, +0, 9 ],
            [ b, +0, 10 ],
            [ b, +0, 11 ],
            [ b, +0, 12 ],
            [ b, +0, 13 ],
            [ b, +0, 14 ],
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

            [ b, 8, undefined ],
            [ b, 8, null ],
            [ b, 8, true ],
            [ b, 8, false ],
            [ b, { valueOf: function () { return 8; } }, { valueOf: function () { return 2; } } ],

            [ b, 4, 0 ],
            [ b, 4, 1 ],
            [ b, 4, 2 ],
            [ b, 4, 3 ],
            [ b, 4, 4 ],
            [ b, 4, 5 ],
            [ b, 4, 6 ],
            [ b, 4, 7 ],
            [ b, 4, 8 ],
            [ b, 4, 9 ],
            [ b, 4, 10 ],
            [ b, 4, 11 ],
            [ b, 4, 12 ],
            [ b, 4, 13 ],
            [ b, 4, 14 ],
            [ b, 4, 15 ],
            [ b, 4, 16 ],

            [ b, 8, 0 ],
            [ b, 8, 1 ],
            [ b, 8, 2 ],
            [ b, 8, 3 ],
            [ b, 8, 4 ],
            [ b, 8, 5 ],
            [ b, 8, 6 ],
            [ b, 8, 7 ],
            [ b, 8, 8 ],
            [ b, 8, 9 ],
            [ b, 8, 10 ],
            [ b, 8, 11 ],
            [ b, 8, 12 ],
            [ b, 8, 13 ],
            [ b, 8, 14 ],
            [ b, 8, 15 ],
            [ b, 8, 16 ],

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

            [ b, 8, 1, 'dummy' ]
        ].forEach(function (arglist) {
            try {
                evalstr = 'new ' + consname + '(';
                for (i = 0; i < arglist.length; i++) {
                    if (i > 0) { evalstr += ', '; }
                    evalstr += 'arglist[' + i + ']';
                }
                evalstr += ')';

                v = eval(evalstr);
                print(arglist, evalstr, v.buffer === b, v.length, v.byteOffset, v.byteLength);
            } catch (e) {
                print(arglist, e.name);
            }
        });
    });
}

try {
    print('TypedArray argument bruteforce test');
    typedArrayArgumentBruteForceTest();
} catch (e) {
    print(e.stack || e);
}

/*===
only constructor call
object [object Float32Array] 3 12 0
TypeError
===*/

function onlyConstructorCallTest() {
    var v;

    // Views must be called as constructor calls.

    v = new Float32Array(3);
    printProps(v);

    try {
        v = Float32Array(3);  // -> TypeError
        printProps(v);   // never here
    } catch (e) {
        print(e.name);
    }
}

try {
    print('only constructor call');
    onlyConstructorCallTest();
} catch (e) {
    print(e.stack || e);
}

/*===
number argument
object [object Float32Array] 16 64 0
object [object ArrayBuffer] 64 64 0
===*/

function numberArgumentTest() {
    var b, v;

    // Number argument creates a new TypedArray with that many elements.
    // Byte count will be a multiple of that.  There's an underlying
    // ArrayBuffer which automatically gets created.

    v = new Float32Array(16);
    printProps(v);
    b = v.buffer;
    printProps(b);
}

try {
    print('number argument');
    numberArgumentTest();
} catch (e) {
    print(e.stack || e);
}

/*===
ArrayBuffer argument
object [object ArrayBuffer] 20 20 0
object [object Uint32Array] 5 20 0
true
RangeError
RangeError
object [object Uint32Array] 4 16 4
true
RangeError
RangeError
===*/

function arrayBufferArgumentTest() {
    var b, v;

    b = new ArrayBuffer(20);
    printProps(b);

    // Without any other arguments, creates a view mapping the whole
    // ArrayBuffer, assuming its length is a proper multiple of the
    // element size.

    v = new Uint32Array(b);
    printProps(v);
    print(v.buffer === b);

    try {
        v = new Float64Array(b);  // 20/8 doesn't divide evenly -> RangeError
        printProps(v);
        print(v.buffer === b);
    } catch (e) {
        print(e.name);
    }

    // Byte offset alone means the rest of the buffer is mapped; the offset
    // must be a multiple of the element size and the buffer slice used must
    // divide evenly.

    try {
        v = new Uint32Array(b, 2);  // offset is not a multiple of 4 -> RangeError
    } catch (e) {
        print(e.name);
    }

    v = new Uint32Array(b, 4);
    printProps(v);
    print(v.buffer === b);

    try {
        // Here the remainder (16 bytes) divides evenly by 8, but the offset
        // is not a multiple of 8 -> RangeError
        v = new Float64Array(b, 4);
    } catch (e) {
        print(e.name);
    }

    try {
        // Here the offset is a multiple of 8, but the remainder (12) doesn't
        // divide evenly -> RangeError.
        v = new Float64Array(b, 8);
    } catch (e) {
        print(e.name);
    }
}

try {
    print('ArrayBuffer argument');
    arrayBufferArgumentTest();
} catch (e) {
    print(e.stack || e);
}

/*===
TypedArray argument
object [object Uint8Array] 4 4 0
object [object Int8Array] 4 4 0
0 127 -128 -1
object
object
false
object [object Int32Array] 4 16 0
object [object Uint8Array] 4 4 0
178 239 255 170
object
object
false
object [object Uint8Array] 4 4 0
object [object Float64Array] 4 32 0
178 239 255 170
object
object
false
object [object Uint8ClampedArray] 4 4 0
0 0 1 127
object
object
false
object [object Int8Array] 4 4 0
0 127 -128 -1
object
object
false
===*/

function typedArrayArgumentTest() {
    // Conceptually a TypedArray argument works just like a plain Array
    // argument: resulting view length depends on element (not byte) length
    // of the input, and values are converted from one TypedArray to another.
    // A new ArrayBuffer is always created (no slice/view).
    //
    // Internally the implementation uses a memcpy() when the elements are
    // safe to byte copy, and explicit coercion otherwise.  See coercion
    // compatibility comments for the TypedArray set() testcase.

    var v1, v2;

    // Uint8 -> Int8 coervcion is byte copy compatible.

    v1 = new Uint8Array(4);
    v1[0] = 0x00;
    v1[1] = 0x7f;
    v1[2] = 0x80;
    v1[3] = 0x1ff;
    printProps(v1);

    v2 = new Int8Array(v1);
    printProps(v2);
    print(v2[0], v2[1], v2[2], v2[3]);

    print(typeof v1.buffer);
    print(typeof v2.buffer);
    print(v1.buffer === v2.buffer);

    // Int32 -> Uint8 coercion is not a byte copy so this causes an explicit
    // conversion internally.

    v1 = new Int32Array(4);
    v1[0] = -12345678;
    v1[1] = 0xdeadbeef;
    v1[2] = -1;
    v1[3] = 0x55aa55aa;
    printProps(v1);

    v2 = new Uint8Array(v1);
    printProps(v2);
    print(v2[0], v2[1], v2[2], v2[3]);

    print(typeof v1.buffer);
    print(typeof v2.buffer);
    print(v1.buffer === v2.buffer);

    // Similarly Uint8 -> Float64 is not a byte copy.

    v1 = new Uint8Array(4);
    v1[0] = -12345678;
    v1[1] = 0xdeadbeef;
    v1[2] = -1;
    v1[3] = 0x55aa55aa;
    printProps(v1);

    v2 = new Float64Array(v1);
    printProps(v2);
    print(v2[0], v2[1], v2[2], v2[3]);

    print(typeof v1.buffer);
    print(typeof v2.buffer);
    print(v1.buffer === v2.buffer);

    // Int8 -> Uint8Clamped is also not a byte copy despite the element size
    // being the same, because e.g. -1 coerces to 0x00.

    v1 = new Int8Array(4);
    v1[0] = -0x80;
    v1[1] = -1;
    v1[2] = 1;
    v1[3] = 0x7f;

    v2 = new Uint8ClampedArray(v1);
    printProps(v2);
    print(v2[0], v2[1], v2[2], v2[3]);

    print(typeof v1.buffer);
    print(typeof v2.buffer);
    print(v1.buffer === v2.buffer);

    // Uint8Clamped -> Int8 -is- a byte copy.

    v1 = new Uint8ClampedArray(4);
    v1[0] = 0x00;
    v1[1] = 0x7f;
    v1[2] = 0x80;
    v1[3] = 0xff;

    v2 = new Int8Array(v1);
    printProps(v2);
    print(v2[0], v2[1], v2[2], v2[3]);

    print(typeof v1.buffer);
    print(typeof v2.buffer);
    print(v1.buffer === v2.buffer);
}

try {
    print('TypedArray argument');
    typedArrayArgumentTest();
} catch (e) {
    print(e.stack || e);
}

/*===
plain Array argument
object [object Uint8ClampedArray] 8 8 0
0 0 0 127 128 255 255 255
object [object Int32Array] 6 24 0
0 -123456789 -100 100 -559038737 0
===*/

function plainArrayArgumentTest() {
    var v;

    v = new Uint8ClampedArray([
        -1000, -1, 0, 0x7f, 0x80, 0xff, 0x100, 1 / 0
    ]);
    printProps(v);
    print(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);

    v = new Int32Array([
        -1 / 0, -123456789, -100, 100, 0x100deadbeef, 1 / 0
    ]);
    printProps(v);
    print(v[0], v[1], v[2], v[3], v[4], v[5]);
}

try {
    print('plain Array argument');
    plainArrayArgumentTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Array-like argument
object [object Uint16Array] 0 0 0
object [object Uint16Array] 3 6 0
123 65535 48879
===*/

function arrayLikeArgumentTest() {
    var v;

    // Array like arguments work as expected: 'length' is used to figure
    // out the length, and elements are then read using number indices.

    v = new Uint16Array({
        '0': 123,
        '1': 321,
        '2': -1
    });  // no 'length' -> zero size result
    printProps(v);

    v = new Uint16Array({
        '0': 123,
        '1': -1,
        '2': 0xdeadbeef,
        '3': 0x12345678,
        length: 3  // index '3' is ignored
    });
    printProps(v);
    print(v[0], v[1], v[2]);  // value coercion
}

try {
    print('Array-like argument');
    arrayLikeArgumentTest();
} catch (e) {
    print(e.stack || e);
}

/*===
string argument
object [object Int16Array] 0 0 0
object [object Int16Array] 12 24 0
object [object Int16Array] 9 18 0
0 0 0 0 0 1 2 3 4
===*/

function stringArgumentTest() {
    // A plain string is now coerced to a number, so 'foo' ends up as a
    // zero size result while '12' produces a 12-element view.

    var v;

    try {
        v = new Int16Array('foo');
        printProps(v);
    } catch (e) {
        print(e.name);
    }

    try {
        v = new Int16Array('12');
        printProps(v);
    } catch (e) {
        print(e.name);
    }

    // But a String object is "Array-like" and "works".  Oddly, because
    // index values are characters (e.g. str[0] = 'f') they mostly coerce
    // to zero, but a character value like '1' will coerce to a non-zero
    // number!
    //
    // This is more accidental than intentional/useful behavior, but
    // test for it anyway as it should fall out of the processing rules.

    try {
        v = new Int16Array(new String('foo\u1234\ucafe1234'));
        printProps(v);
        print(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8]);
    } catch (e) {
        print(e.name);
    }
}

try {
    print('string argument');
    stringArgumentTest();
} catch (e) {
    print(e.stack || e);
}

/*===
plain buffer argument
object |00666f6fff| |00666f6fff|
|00666f6fff| |fe666f6fff| [object ArrayBuffer]
object |00666f6fff| |000066006f006f00ff00|
|00666f6fff| |feca66006f006f00ff00| [object ArrayBuffer]
buffer |00666f6fff| |00666f6fff|
|00666f6fff| |fe666f6fff| [object ArrayBuffer]
buffer |00666f6fff| |000066006f006f00ff00|
|00666f6fff| |feca66006f006f00ff00| [object ArrayBuffer]
===*/

/* Since Duktape 1.4.0 a plain Duktape buffer is accepted similarly to a
 * Duktape.Buffer.  Behavior in Duktape 1.3.0 is a bit confusing:
 *
 *     duk> u8 = new Uint8Array(new Duktape.Buffer('foobar'))
 *     = [object Uint8Array]
 *     duk> Duktape.enc('jx', u8)
 *     = |666f6f626172|
 *     duk> u8 = new Uint8Array(Duktape.Buffer('foobar'))
 *     = [object Uint8Array]
 *     duk> Duktape.enc('jx', u8)
 *     = ||
 */

function plainBufferArgumentTest() {
    var buf, view;

    function test(buf, view) {
        print(typeof buf, Duktape.enc('jx', buf), Duktape.enc('jx', view));
        view[0] = 0xcafe;  // demonstrate view does not share underlying buffer
        print(Duktape.enc('jx', buf), Duktape.enc('jx', view), Object.prototype.toString.call(view.buffer));
    }

    buf = new Duktape.Buffer(Duktape.dec('hex', '00666f6fff'));
    view = new Uint8Array(buf);
    test(buf, view);

    buf = new Duktape.Buffer(Duktape.dec('hex', '00666f6fff'));
    view = new Int16Array(buf);
    test(buf, view);

    buf = Duktape.dec('hex', '00666f6fff');
    view = new Uint8Array(buf);
    test(buf, view);

    buf = Duktape.dec('hex', '00666f6fff');
    view = new Int16Array(buf);
    test(buf, view);
}

try {
    print('plain buffer argument');
    plainBufferArgumentTest();
} catch (e) {
    print(e.stack || e);
}
