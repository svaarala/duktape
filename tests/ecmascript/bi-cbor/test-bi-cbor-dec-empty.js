/*===
TypeError
===*/

// Empty input is a decode error, TypeError used now.  cbor-js triggers a
// RangeError from reading out of bounds of a DataView.
try {
    CBOR.decode(new Uint8Array([]).buffer);
    print('never here');
} catch (e) {
    print(e.name);
    //print(e.stack || e);
}
