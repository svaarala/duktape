/*
 *  Typeof
 */

/*@include util-typedarray.js@*/

/*===
typeof test
[object ArrayBuffer] object
[object DataView] object
[object Int8Array] object
[object Uint8Array] object
[object Uint8ClampedArray] object
[object Int16Array] object
[object Uint16Array] object
[object Int32Array] object
[object Uint32Array] object
[object Float32Array] object
[object Float64Array] object
===*/

function typeofTest() {
    var objs = getTestObjectList();
    objs.forEach(function (b) {
        print(Object.prototype.toString.call(b), typeof b);
    });
}

try {
    print('typeof test');
    typeofTest();
} catch (e) {
    print(e.stack || e);
}
