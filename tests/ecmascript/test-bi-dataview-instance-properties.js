/*
 *  DataView instance properties
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
DataView instance properties test
buffer true object [object ArrayBuffer]
byteOffset true number 13
byteLength true number 17
BYTES_PER_ELEMENT false undefined undefined
length false undefined undefined
0 false undefined undefined
1 false undefined undefined
2 false undefined undefined
3 false undefined undefined
4 false undefined undefined
5 false undefined undefined
6 false undefined undefined
7 false undefined undefined
8 false undefined undefined
9 false undefined undefined
10 false undefined undefined
11 false undefined undefined
12 false undefined undefined
13 false undefined undefined
14 false undefined undefined
15 false undefined undefined
16 false undefined undefined
17 false undefined undefined
18 false undefined undefined
getInt8 true function function
getInt16 true function function
getUint16 true function function
getInt32 true function function
getUint32 true function function
getFloat32 true function function
getFloat64 true function function
setInt8 true function function
setUint8 true function function
setInt16 true function function
setUint16 true function function
setInt32 true function function
setUint32 true function function
setFloat32 true function function
setFloat64 true function function
true
===*/

function dataViewInstancePropertiesTest() {
    var props = [
        'buffer',              // from ArrayBufferView
        'byteOffset',          // from ArrayBufferView
        'byteLength',          // from ArrayBufferView
        'BYTES_PER_ELEMENT',   // Duktape specific
        'length',              // Duktape specific
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '10', '11', '12', '13', '14', '15', '16',
        '17', '18',            // 0 to 16 from view, 17 and 18 don't exist

        'getInt8',             // from DataView
        'getInt16',            // from DataView
        'getUint16',           // from DataView
        'getInt32',            // from DataView
        'getUint32',           // from DataView
        'getFloat32',          // from DataView
        'getFloat64',          // from DataView
        'setInt8',             // from DataView
        'setUint8',            // from DataView
        'setInt16',            // from DataView
        'setUint16',           // from DataView
        'setInt32',            // from DataView
        'setUint32',           // from DataView
        'setFloat32',          // from DataView
        'setFloat64'           // from DataView
    ];

    props.forEach(function (propname) {
        try {
            var buf = new ArrayBuffer(32);
            var view = new DataView(buf, 13, 17);
            var val = view[propname];
            print(propname, propname in view, typeof val, encValue(val));
        } catch (e) {
            print(e.stack || e);
        }
    });

    var buf = new ArrayBuffer(16);
    var view = new DataView(buf);
    print(view.buffer === buf);
}

try {
    print('DataView instance properties test');
    dataViewInstancePropertiesTest();
} catch (e) {
    print(e.stack || e);
}
