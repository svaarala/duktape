/*
 *  Properties of ArrayBuffer.prototype
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
ArrayBuffer prototype properties test
slice true function function
isView false undefined undefined
constructor true function function
true
===*/

function arrayBufferPrototypePropertiesTest() {
    var props = [
        'slice',
        'isView',  // not present, provided by ArrayBuffer constructor
        'constructor'
    ];

    props.forEach(function (propname) {
        try {
            var obj = ArrayBuffer.prototype;
            var val = obj[propname];
            print(propname, propname in obj, typeof val, encValue(val));
        } catch (e) {
            print(e.stack || e);
        }
    });

    print(ArrayBuffer.prototype.constructor === ArrayBuffer);
}

try {
    print('ArrayBuffer prototype properties test');
    arrayBufferPrototypePropertiesTest();
} catch (e) {
    print(e.stack || e);
}
