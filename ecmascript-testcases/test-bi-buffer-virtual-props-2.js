/*
 *  New virtual properties were added in Duktape 1.3 to Duktape.Buffer
 *  objects; they were also added to plain buffer values for consistency.
 */

/*---
{
    "custom": true
}
---*/

/*===
plain
4
4
0
1
object
6
6
0
1
===*/


function bufferVirtualPropertiesTest() {
    var buf_plain, buf_object;

    print('plain');
    buf_plain = Duktape.dec('hex', 'deadbeef');
    print(buf_plain.length);  // already in Duktape 1.2
    print(buf_plain.byteLength);  // new
    print(buf_plain.byteOffset);  // new
    print(buf_plain.BYTES_PER_ELEMENT);  // new

    print('object');
    buf_object = new Duktape.Buffer('foobar');
    print(buf_object.length);  // already in Duktape 1.2
    print(buf_object.byteLength);  // new
    print(buf_object.byteOffset);  // new
    print(buf_object.BYTES_PER_ELEMENT);  // new
}

try {
    bufferVirtualPropertiesTest();
} catch (e) {
    print(e.stack || e);
}
