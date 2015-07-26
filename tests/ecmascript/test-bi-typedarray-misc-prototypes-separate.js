/*
 *  Prototype objects are not shared.
 */

/*@include util-typedarray.js@*/

/*===
1000000000000000000000
0100000000000000000000
0010000000000000000000
0001000000000000000000
0000100000000000000000
0000010000000000000000
0000001000000000000000
0000000100000000000000
0000000010000000000000
0000000001000000000000
0000000000100000000000
0000000000010000000000
0000000000001000000000
0000000000000100000000
0000000000000010000000
0000000000000001000000
0000000000000000100000
0000000000000000010000
0000000000000000001000
0000000000000000000100
0000000000000000000010
0000000000000000000001
===*/

var objects = [
    ArrayBuffer,
    ArrayBuffer.prototype,
    DataView,
    DataView.prototype,
    Int8Array,
    Int8Array.prototype,
    Uint8Array,
    Uint8Array.prototype,
    Uint8ClampedArray,
    Uint8ClampedArray.prototype,
    Int16Array,
    Int16Array.prototype,
    Uint16Array,
    Uint16Array.prototype,
    Int32Array,
    Int32Array.prototype,
    Uint32Array,
    Uint32Array.prototype,
    Float32Array,
    Float32Array.prototype,
    Float64Array,
    Float64Array.prototype
];

function testSeparatePrototypes() {
    var i, j;
    var tmp;

    for (i = 0; i < objects.length; i++) {
        tmp = '';
        for (j = 0; j < objects.length; j++) {
            tmp += (objects[i] === objects[j] ? '1' : '0');
        }
        print(tmp);
    }
}

try {
    testSeparatePrototypes();
} catch (e) {
    print(e.stack || e);
}
