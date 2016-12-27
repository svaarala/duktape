/*
 *  Properties of ArrayBuffer instances
 */

function encValue(v) {
    if (typeof v === 'function') { return 'function'; }
    return String(v);
}

/*===
ArrayBuffer instance properties test
byteLength true number 2
byteOffset false undefined undefined
length false undefined undefined
BYTES_PER_ELEMENT false undefined undefined
slice true function function
isView false undefined undefined
0 false undefined undefined
1 false undefined undefined
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
        '0', '1', '2'   // indexed props not present
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
