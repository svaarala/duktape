/*===
[1,2,3]
done
===*/

// While 64-bit integers might not be supported, a 64-bit tag must still be
// at least ignored.
var t = CBOR.decode(new Uint8Array([
    0xdb, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x83, 0x01, 0x02, 0x03
]));
print(JSON.stringify(t));

print('done');
