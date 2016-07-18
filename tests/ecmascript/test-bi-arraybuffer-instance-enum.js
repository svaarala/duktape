/*
 *  Enumerating an ArrayBuffer instance
 */

/*---
{
    "custom": true
}
---*/

/*===
0
Object.getOwnPropertyNames: length
Object.getOwnPropertyNames: byteLength
Object.getOwnPropertyNames: byteOffset
Object.getOwnPropertyNames: BYTES_PER_ELEMENT
===*/

function arrayBufferEnumTest() {
    var buf = new ArrayBuffer(8);

    // The ArrayBuffer index keys are non-standard and they can be read/written.
    // However, the keys don't enumerate or JSON serialize.
    print(buf[0]);

    for (var k in buf) {
        print('for-in:', k);
    }

    Object.keys(buf).forEach(function (k) {
        print('Object.keys:', k);
    });

    Object.getOwnPropertyNames(buf).forEach(function (k) {
        print('Object.getOwnPropertyNames:', k);
    });
}

try {
    arrayBufferEnumTest();
} catch (e) {
    print(e.stack || e);
}
