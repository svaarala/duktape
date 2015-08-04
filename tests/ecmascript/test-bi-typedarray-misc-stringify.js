/*
 *  Stringify
 */

/*@include util-typedarray.js@*/

/*===
stringify test
[object ArrayBuffer] [object ArrayBuffer] [object ArrayBuffer]
[object DataView] [object DataView] [object DataView]
[object Int8Array] [object Int8Array] [object Int8Array]
[object Uint8Array] [object Uint8Array] [object Uint8Array]
[object Uint8ClampedArray] [object Uint8ClampedArray] [object Uint8ClampedArray]
[object Int16Array] [object Int16Array] [object Int16Array]
[object Uint16Array] [object Uint16Array] [object Uint16Array]
[object Int32Array] [object Int32Array] [object Int32Array]
[object Uint32Array] [object Uint32Array] [object Uint32Array]
[object Float32Array] [object Float32Array] [object Float32Array]
[object Float64Array] [object Float64Array] [object Float64Array]
===*/

function stringifyTest() {
    var objs = getTestObjectList();

    objs.forEach(function (b) {
        print(Object.prototype.toString.call(b), b.toString(), String(b));
    });
}

try {
    print('stringify test');
    stringifyTest();
} catch (e) {
    print(e.stack || e);
}
