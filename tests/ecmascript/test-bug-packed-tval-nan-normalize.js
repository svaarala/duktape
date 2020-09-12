/*---
{
    "custom": true
}
---*/

/*===
NaN
done
===*/

try {
    var val = CBOR.decode(new Uint8Array([ 0xfb, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 ]));
    print(val);
} catch (e) {
    print(e.stack || e);
}

print('done');
