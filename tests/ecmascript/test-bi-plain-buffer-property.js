/*@include util-buffer.js@*/

/*===
property test
16
48
16
0
1
[object ArrayBuffer]
[object ArrayBuffer]
false
===*/

function propertyTest() {
    var pb = createPlainBuffer('0123456789abcdef');
    var a1, a2;

    // Uint8Array .length and index properties are virtual own properties.
    print(pb.length);
    print(pb[0]);

    // .byteOffset, .byteLength, .BYTES_PER_ELEMENT are inherited accessors.
    print(pb.byteLength);
    print(pb.byteOffset);
    print(pb.BYTES_PER_ELEMENT);

    // .buffer is an inherited accessor.  It creates a new ArrayBuffer every
    // time the property is read: there's no place to cache the created
    // ArrayBuffer.
    a1 = pb.buffer;
    a2 = pb.buffer;
    print(Object.prototype.toString.call(a1));
    print(Object.prototype.toString.call(a2));
    print(a1 === a2);
}

try {
    print('property test');
    propertyTest();
} catch (e) {
    print(e.stack || e);
}
