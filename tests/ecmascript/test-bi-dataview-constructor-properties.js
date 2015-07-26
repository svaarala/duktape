/*
 *  DataView constructor properties
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
DataView constructor properties test
name true string DataView
length true number 3
prototype true object [object Object]
true
===*/

function dataViewConstructorPropertiesTest() {
    var props = [
        'name',
        'length',
        'prototype'
    ];

    props.forEach(function (propname) {
        try {
            var obj = DataView;
            var val = obj[propname];
            print(propname, propname in obj, typeof val, encValue(val));
        } catch (e) {
            print(e.stack || e);
        }
    });

    print(DataView.prototype.constructor === DataView);
}

try {
    print('DataView constructor properties test');
    dataViewConstructorPropertiesTest();
} catch (e) {
    print(e.stack || e);
}
