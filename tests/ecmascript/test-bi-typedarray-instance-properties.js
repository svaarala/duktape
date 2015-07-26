/*
 *  TypedArray instance properties
 */

/*---
{
    "custom": true
}
---*/

function encValue(v) {
    if (typeof v === 'function') { return 'function'; }
    return String(v);
}

/*===
TypedArray instance properties test
Int8Array buffer true object [object ArrayBuffer]
Int8Array byteOffset true number 0
Int8Array byteLength true number 2
Int8Array BYTES_PER_ELEMENT true number 1
Int8Array length true number 2
Int8Array set true function function
Int8Array subarray true function function
Int8Array 0 true number 0
Int8Array 1 true number 0
Int8Array 2 false undefined undefined
Uint8Array buffer true object [object ArrayBuffer]
Uint8Array byteOffset true number 0
Uint8Array byteLength true number 2
Uint8Array BYTES_PER_ELEMENT true number 1
Uint8Array length true number 2
Uint8Array set true function function
Uint8Array subarray true function function
Uint8Array 0 true number 0
Uint8Array 1 true number 0
Uint8Array 2 false undefined undefined
Uint8ClampedArray buffer true object [object ArrayBuffer]
Uint8ClampedArray byteOffset true number 0
Uint8ClampedArray byteLength true number 2
Uint8ClampedArray BYTES_PER_ELEMENT true number 1
Uint8ClampedArray length true number 2
Uint8ClampedArray set true function function
Uint8ClampedArray subarray true function function
Uint8ClampedArray 0 true number 0
Uint8ClampedArray 1 true number 0
Uint8ClampedArray 2 false undefined undefined
Int16Array buffer true object [object ArrayBuffer]
Int16Array byteOffset true number 0
Int16Array byteLength true number 4
Int16Array BYTES_PER_ELEMENT true number 2
Int16Array length true number 2
Int16Array set true function function
Int16Array subarray true function function
Int16Array 0 true number 0
Int16Array 1 true number 0
Int16Array 2 false undefined undefined
Uint16Array buffer true object [object ArrayBuffer]
Uint16Array byteOffset true number 0
Uint16Array byteLength true number 4
Uint16Array BYTES_PER_ELEMENT true number 2
Uint16Array length true number 2
Uint16Array set true function function
Uint16Array subarray true function function
Uint16Array 0 true number 0
Uint16Array 1 true number 0
Uint16Array 2 false undefined undefined
Int32Array buffer true object [object ArrayBuffer]
Int32Array byteOffset true number 0
Int32Array byteLength true number 8
Int32Array BYTES_PER_ELEMENT true number 4
Int32Array length true number 2
Int32Array set true function function
Int32Array subarray true function function
Int32Array 0 true number 0
Int32Array 1 true number 0
Int32Array 2 false undefined undefined
Uint32Array buffer true object [object ArrayBuffer]
Uint32Array byteOffset true number 0
Uint32Array byteLength true number 8
Uint32Array BYTES_PER_ELEMENT true number 4
Uint32Array length true number 2
Uint32Array set true function function
Uint32Array subarray true function function
Uint32Array 0 true number 0
Uint32Array 1 true number 0
Uint32Array 2 false undefined undefined
Float32Array buffer true object [object ArrayBuffer]
Float32Array byteOffset true number 0
Float32Array byteLength true number 8
Float32Array BYTES_PER_ELEMENT true number 4
Float32Array length true number 2
Float32Array set true function function
Float32Array subarray true function function
Float32Array 0 true number 0
Float32Array 1 true number 0
Float32Array 2 false undefined undefined
Float64Array buffer true object [object ArrayBuffer]
Float64Array byteOffset true number 0
Float64Array byteLength true number 16
Float64Array BYTES_PER_ELEMENT true number 8
Float64Array length true number 2
Float64Array set true function function
Float64Array subarray true function function
Float64Array 0 true number 0
Float64Array 1 true number 0
Float64Array 2 false undefined undefined
===*/

function typedArrayInstancePropertiesTest() {
    var views = [
        'Int8Array', 'Uint8Array', 'Uint8ClampedArray',
        'Int16Array', 'Uint16Array',
        'Int32Array', 'Uint32Array',
        'Float32Array', 'Float64Array'
    ];
    var props = [
        'buffer',              // from ArrayBufferView
        'byteOffset',          // from ArrayBufferView
        'byteLength',          // from ArrayBufferView
        'BYTES_PER_ELEMENT',   // from TypedArray
        'length',              // from TypedArray
        'set',                 // from TypedArray
        'subarray',            // from TypedArray
        '0', '1', '2'          // '0' and '1' from TypedArray, '2' doesn't exist
    ];

    views.forEach(function (viewname) {
        props.forEach(function (propname) {
            try {
                var view = eval('new ' + viewname + '(2)');
                var val = view[propname];
                print(viewname, propname, propname in view, typeof val, encValue(val));
            } catch (e) {
                print(e.stack || e);
            }
        });
    });
}

try {
    print('TypedArray instance properties test');
    typedArrayInstancePropertiesTest();
} catch (e) {
    print(e.stack || e);
}
