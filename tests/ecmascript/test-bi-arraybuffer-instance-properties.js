/*
 *  Properties of ArrayBuffer instances
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
ArrayBuffer instance properties test
byteLength true number 2
byteOffset true number 0
length true number 2
BYTES_PER_ELEMENT true number 1
slice true function function
isView false undefined undefined
0 true number 0
1 true number 0
2 false undefined undefined
===*/

function arrayBufferInstancePropertiesTest() {
    var props = [
        'byteLength',
        'byteOffset',   // not present in spec, Duktape provides
        'length',       // not present in spec, Duktape provides
        'BYTES_PER_ELEMENT',
        'slice',
        'isView',       // not present for ArrayBuffer instances, only ArrayBuffer constructor
        '0', '1', '2'   // indexed props not present in spec, Duktape provides [0,length[
    ];

    props.forEach(function (propname) {
        try {
            var buf = new ArrayBuffer(2);
            var val = buf[propname];
            print(propname, propname in buf, typeof val, encValue(val));
        } catch (e) {
            print(e.stack || e);
        }
    });
}

try {
    print('ArrayBuffer instance properties test');
    arrayBufferInstancePropertiesTest();
} catch (e) {
    print(e.stack || e);
}
