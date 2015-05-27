/*
 *  Node.js Buffer instance properties
 */

function encValue(x) {
    if (typeof x === 'function') { return 'function'; }
    return String(x);
}

/*===
Node.js Buffer instanceproperties test
length true number 4
0 true number 0
1 true number 0
2 true number 0
3 true number 0
4 false undefined undefined
byteLength true number 4
byteOffset true number 0
BYTES_PER_ELEMENT true number 1
===*/

function nodejsBufferInstancePropertiesTest() {
    var props = [
        'length',
        '0', '1', '2', '3',   // present
        '4',                  // not present

        'byteLength',         // Duktape specific
        'byteOffset',         // Duktape specific
        'BYTES_PER_ELEMENT',  // Duktape specific
    ];

    props.forEach(function (propname) {
        var obj = new Buffer(4);
        var val = obj[propname];
        print(propname, propname in obj, typeof val, encValue(val));
    });
}

try {
    print('Node.js Buffer instanceproperties test');
    nodejsBufferInstancePropertiesTest();
} catch (e) {
    print(e.stack || e);
}
