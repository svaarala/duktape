/*
 *  Enumerating a DataView instance
 */

/*---
{
    "custom": true
}
---*/

/*===
===*/

function dataViewEnumTest() {
    var buf = new ArrayBuffer(64);
    var view = new DataView(buf, 11, 8);

    // Nothing enumerates with for-in, and nothing is returned for
    // Object.keys() or Object.getOwnPropertyNames().

    for (var k in view) {
        print('for-in:', k);
    }

    Object.keys(view).forEach(function (k) {
        print('Object.keys:', k);
    });

    Object.getOwnPropertyNames(view).forEach(function (k) {
        print('Object.getOwnPropertyNames:', k);
    });
}

try {
    dataViewEnumTest();
} catch (e) {
    print(e.stack || e);
}
