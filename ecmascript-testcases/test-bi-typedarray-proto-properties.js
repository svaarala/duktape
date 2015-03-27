/*
 *  TypedArray prototype properties
 */

/*@include util-typedarray.js@*/

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
TypedArray prototype properties test
Int8Array
set true function function
subarray true function function
constructor true function function
true
Uint8Array
set true function function
subarray true function function
constructor true function function
true
Uint8ClampedArray
set true function function
subarray true function function
constructor true function function
true
Int16Array
set true function function
subarray true function function
constructor true function function
true
Uint16Array
set true function function
subarray true function function
constructor true function function
true
Int32Array
set true function function
subarray true function function
constructor true function function
true
Uint32Array
set true function function
subarray true function function
constructor true function function
true
Float32Array
set true function function
subarray true function function
constructor true function function
true
Float64Array
set true function function
subarray true function function
constructor true function function
true
===*/

function typedArrayPrototypePropertiesTest() {
    var views = [
        'Int8Array', 'Uint8Array', 'Uint8ClampedArray',
        'Int16Array', 'Uint16Array',
        'Int32Array', 'Uint32Array',
        'Float32Array', 'Float64Array'
    ];
    var props = [
        'set',
        'subarray',
        'constructor'
    ];

    views.forEach(function (viewname) {
        print(viewname);

        var obj = eval(viewname + '.prototype');

        props.forEach(function (propname) {
            try {
                var val = obj[propname];
                print(propname, propname in obj, typeof val, encValue(val));
            } catch (e) {
                print(e.stack || e);
            }
        });

        print(obj.constructor === eval(viewname));
    });
}

try {
    print('TypedArray prototype properties test');
    typedArrayPrototypePropertiesTest();
} catch (e) {
    print(e.stack || e);
}
