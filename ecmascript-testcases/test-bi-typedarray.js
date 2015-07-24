/*
 *  TypedArray tests
 *
 *  - https://www.khronos.org/registry/typedarray/specs/latest/
 *
 *  - svn co -r 30720 https://cvs.khronos.org/svn/repos/registry/trunk/public/typedarray
 *
 *  - http://www.ecma-international.org/ecma-262/6.0/
 *
 *  TypedArray views have platform specific endianness behavior which must
 *  be taken into account in testcases.
 */

/*@include util-typedarray.js@*/

/*---
{
    "custom": true
}
---*/

/*===
prototype chain and own property test
[object ArrayBuffer] -> ArrayBuffer.prototype -> Object.prototype
BYTES_PER_ELEMENT 1
byteLength 16
byteOffset 0
length 16
[object DataView] -> DataView.prototype -> Object.prototype
BYTES_PER_ELEMENT 1
buffer [object ArrayBuffer]
byteLength 16
byteOffset 0
length 16
[object Int8Array] -> Int8Array.prototype -> TypedArray.prototype -> Object.prototype
BYTES_PER_ELEMENT 1
buffer [object ArrayBuffer]
byteLength 16
byteOffset 0
length 16
[object Uint8Array] -> Uint8Array.prototype -> TypedArray.prototype -> Object.prototype
BYTES_PER_ELEMENT 1
buffer [object ArrayBuffer]
byteLength 16
byteOffset 0
length 16
[object Uint8ClampedArray] -> Uint8ClampedArray.prototype -> TypedArray.prototype -> Object.prototype
BYTES_PER_ELEMENT 1
buffer [object ArrayBuffer]
byteLength 16
byteOffset 0
length 16
[object Int16Array] -> Int16Array.prototype -> TypedArray.prototype -> Object.prototype
BYTES_PER_ELEMENT 2
buffer [object ArrayBuffer]
byteLength 16
byteOffset 0
length 8
[object Uint16Array] -> Uint16Array.prototype -> TypedArray.prototype -> Object.prototype
BYTES_PER_ELEMENT 2
buffer [object ArrayBuffer]
byteLength 16
byteOffset 0
length 8
[object Int32Array] -> Int32Array.prototype -> TypedArray.prototype -> Object.prototype
BYTES_PER_ELEMENT 4
buffer [object ArrayBuffer]
byteLength 16
byteOffset 0
length 4
[object Uint32Array] -> Uint32Array.prototype -> TypedArray.prototype -> Object.prototype
BYTES_PER_ELEMENT 4
buffer [object ArrayBuffer]
byteLength 16
byteOffset 0
length 4
[object Float32Array] -> Float32Array.prototype -> TypedArray.prototype -> Object.prototype
BYTES_PER_ELEMENT 4
buffer [object ArrayBuffer]
byteLength 16
byteOffset 0
length 4
[object Float64Array] -> Float64Array.prototype -> TypedArray.prototype -> Object.prototype
BYTES_PER_ELEMENT 8
buffer [object ArrayBuffer]
byteLength 16
byteOffset 0
length 2
===*/

function prototypeChainAndOwnPropertyTest() {
    var objs = getTestObjectList();

    objs.forEach(function (b) {
        printPrototypeChain(b);
        dumpOwnNonIndexProperties(b, true);
    });
}

try {
    print('prototype chain and own property test');
    prototypeChainAndOwnPropertyTest();
} catch (e) {
    print(e.stack || e);
}

/*
 *  Summary of methods and properties
 */

// new ArrayBuffer(length)
// ArrayBuffer(length) -> TypeError
// ArrayBuffer.isView()
// buf.slice(begin, [end])
// buf.byteLength
// buf.byteOffset (Duktape specific)
// buf.length (Duktape specific, same as .byteLength)
// buf.BYTES_PER_ELEMENT (Duktape specific; 1)
// buf[index] (Duktape specific)

// new TypedArray(length)
// new TypedArray(TypedArray array)
// new TypedArray(type[] array)
// new TypedArray(ArrayBuffer, [byteOffset], [length])
// TypedArray(length) -> TypeError
// TypedArray(TypedArray array) -> TypeError
// TypedArray(type[] array) -> TypeError
// TypedArray(ArrayBuffer, [byteOffset], [length]) -> TypeError
// typedArray.buffer
// typedArray.byteOffset
// typedArray.byteLength
// typedArray.length
// typedArray.BYTES_PER_ELEMENT
// typedArray[index]
// typedArray.set(typedArray, [offset])
// typedArray.set(type[] array, [offset])
// typedArray.subarray(begin, [end])

// new DataView(arrayBuffer, [byteOffset], [byteLength])
// DataView(arrayBuffer, [byteOffset], [byteLength]) -> TypeError
// dataView.buffer
// dataView.byteOffset
// dataView.byteLength
// dataView.length (Duktape specific, same as. byteLength)
// dataView[index] (Duktape specific; Uint8)
// dataView.BYTES_PER_ELEMENT (Duktape specific; 1)
// dataView.getInt8(byteOffset)
// dataView.getInt16(byteOffset, [littleEndian])
// dataView.getUint16(byteOffset, [littleEndian])
// dataView.getInt32(byteOffset, [littleEndian])
// dataView.getUint32(byteOffset, [littleEndian])
// dataView.getFloat32(byteOffset, [littleEndian])
// dataView.getFloat64(byteOffset, [littleEndian])
// dataView.setInt8(byteOffset, value)
// dataView.setUint8(byteOffset, value)
// dataView.setInt16(byteOffset, value, [littleEndian])
// dataView.setUint16(byteOffset, value, [littleEndian])
// dataView.setInt32(byteOffset, value, [littleEndian])
// dataView.setUint32(byteOffset, value, [littleEndian])
// dataView.setFloat32(byteOffset, value, [littleEndian])
// dataView.setFloat64(byteOffset, value, [littleEndian])
