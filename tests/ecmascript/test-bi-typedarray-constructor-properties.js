/*
 *  TypedArray constructor properties
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
TypedArray constructor properties test
Int8Array name true string Int8Array
Int8Array length true number 3
Int8Array prototype true object [object Object]
Int8Array BYTES_PER_ELEMENT true number 1
true
Uint8Array name true string Uint8Array
Uint8Array length true number 3
Uint8Array prototype true object [object Object]
Uint8Array BYTES_PER_ELEMENT true number 1
true
Uint8ClampedArray name true string Uint8ClampedArray
Uint8ClampedArray length true number 3
Uint8ClampedArray prototype true object [object Object]
Uint8ClampedArray BYTES_PER_ELEMENT true number 1
true
Int16Array name true string Int16Array
Int16Array length true number 3
Int16Array prototype true object [object Object]
Int16Array BYTES_PER_ELEMENT true number 2
true
Uint16Array name true string Uint16Array
Uint16Array length true number 3
Uint16Array prototype true object [object Object]
Uint16Array BYTES_PER_ELEMENT true number 2
true
Int32Array name true string Int32Array
Int32Array length true number 3
Int32Array prototype true object [object Object]
Int32Array BYTES_PER_ELEMENT true number 4
true
Uint32Array name true string Uint32Array
Uint32Array length true number 3
Uint32Array prototype true object [object Object]
Uint32Array BYTES_PER_ELEMENT true number 4
true
Float32Array name true string Float32Array
Float32Array length true number 3
Float32Array prototype true object [object Object]
Float32Array BYTES_PER_ELEMENT true number 4
true
Float64Array name true string Float64Array
Float64Array length true number 3
Float64Array prototype true object [object Object]
Float64Array BYTES_PER_ELEMENT true number 8
true
===*/

function typedArrayConstructorPropertiesTest() {
    var props = [
        'name',
        'length',
        'prototype',

        // This is not readily apparent in Khronos specification because
        // this property is listed as a member of TypedArray instance.
        // But the example in Section 10 uses "Float32.BYTES_PER_ELEMENT"
        // and V8 also provides this.

        'BYTES_PER_ELEMENT'
    ];

    var consNames = [
        'Int8Array', 'Uint8Array', 'Uint8ClampedArray',
        'Int16Array', 'Uint16Array',
        'Int32Array', 'Uint32Array',
        'Float32Array', 'Float64Array'
    ];

    consNames.forEach(function (consname) {
        props.forEach(function (propname) {
            try {
                var obj = eval(consname);
                var val = obj[propname];
                print(consname, propname, propname in obj, typeof val, encValue(val));
            } catch (e) {
                print(e.stack || e);
            }
        });

        print(eval(consname).prototype.constructor === eval(consname));
    });
}

try {
    print('TypedArray constructor properties test');
    typedArrayConstructorPropertiesTest();
} catch (e) {
    print(e.stack || e);
}
