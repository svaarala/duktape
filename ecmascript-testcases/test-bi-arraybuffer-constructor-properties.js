/*
 *  ArrayBuffer constructor properties
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
ArrayBuffer constructor properties test
name true string ArrayBuffer
length true number 1
prototype true object [object Object]
isView true function function
true
===*/

function arrayBufferConstructorPropertiesTest() {
    var props = [
        'name',
        'length',
        'prototype',
        'isView'
    ];

    props.forEach(function (propname) {
        try {
            var obj = ArrayBuffer;
            var val = obj[propname];
            print(propname, propname in obj, typeof val, encValue(val));
        } catch (e) {
            print(e.stack || e);
        }
    });

    print(ArrayBuffer.prototype.constructor === ArrayBuffer);
}

try {
    print('ArrayBuffer constructor properties test');
    arrayBufferConstructorPropertiesTest();
} catch (e) {
    print(e.stack || e);
}
