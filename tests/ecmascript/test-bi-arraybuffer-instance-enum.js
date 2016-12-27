/*
 *  Enumerating an ArrayBuffer instance
 */

/*---
{
    "custom": true
}
---*/

/*===
undefined
done
===*/

function arrayBufferEnumTest() {
    var buf = new ArrayBuffer(8);

    // In Duktape 2.x ArrayBuffers don't have non-standard index properties.
    // Nothing gets enumerated for these calls (even .byteLength is inherited).

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

    print('done');
}

try {
    arrayBufferEnumTest();
} catch (e) {
    print(e.stack || e);
}
