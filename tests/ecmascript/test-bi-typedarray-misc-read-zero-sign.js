/*
 *  Specific test that zero value read back is always positive zero for integer
 *  coerced views.  For floating point views negative zero must be preserved.
 */

/*@include util-typedarray.js@*/

/*---
{
    "custom": true
}
---*/

/*===
zero sign test
[object ArrayBuffer] number +0
[object DataView] number +0
[object Int8Array] number +0
[object Uint8Array] number +0
[object Uint8ClampedArray] number +0
[object Int16Array] number +0
[object Uint16Array] number +0
[object Int32Array] number +0
[object Uint32Array] number +0
[object Float32Array] number -0
[object Float64Array] number -0
===*/

function zeroSignTest() {
    var objs = getTestObjectList();

    // Behavior for ArrayBuffer and DataView (virtual indices backed by an
    // Uint8 view) is Duktape specific.  V8, for instance, will create concrete
    // properties which appears to preserve sign.

    objs.forEach(function (b) {
        b[0] = -0;
        print(String(b), typeof b[0], (1 / b[0] > 0) ? '+0' : '-0');
    });
}

try {
    print('zero sign test');
    zeroSignTest();
} catch (e) {
    print(e.stack || e);
}
