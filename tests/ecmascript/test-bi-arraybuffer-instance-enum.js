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
Object.getOwnPropertyNames: 0
Object.getOwnPropertyNames: 1
Object.getOwnPropertyNames: 2
Object.getOwnPropertyNames: 3
Object.getOwnPropertyNames: 4
Object.getOwnPropertyNames: 5
Object.getOwnPropertyNames: 6
Object.getOwnPropertyNames: 7
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

    // The index keys (and other non-enumerated keys) are included in
    // Object.getOwnPropertyNames() output.
    Object.getOwnPropertyNames(buf).forEach(function (k) {
        print('Object.getOwnPropertyNames:', k);
    });
}

try {
    arrayBufferEnumTest();
} catch (e) {
    print(e.stack || e);
}
