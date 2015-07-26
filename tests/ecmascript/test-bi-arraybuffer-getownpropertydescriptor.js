/*
 *  Duktape ArrayBuffer/view virtual properties can be read with
 *  Object.getOwnPropertyDescriptor().
 */

/*@include util-typedarray.js@*/

/*---
{
    "custom": true
}
---*/

/*===
u16 val: length 4
u16 pd: length 4 false false false
i16 val: length 4
i16 pd: length 4 false false false
u16 val: byteLength 8
u16 pd: byteLength 8 false false false
i16 val: byteLength 8
i16 pd: byteLength 8 false false false
u16 val: byteOffset 0
u16 pd: byteOffset 0 false false false
i16 val: byteOffset 0
i16 pd: byteOffset 0 false false false
u16 val: BYTES_PER_ELEMENT 2
u16 pd: BYTES_PER_ELEMENT 2 false false false
i16 val: BYTES_PER_ELEMENT 2
i16 pd: BYTES_PER_ELEMENT 2 false false false
u16 val: 0 65535
u16 pd: 0 65535 true true false
i16 val: 0 -1
i16 pd: 0 65535 true true false
u16 val: 1 32896
u16 pd: 1 32896 true true false
i16 val: 1 -32640
i16 pd: 1 32896 true true false
u16 val: 2 4369
u16 pd: 2 4369 true true false
i16 val: 2 4369
i16 pd: 2 4369 true true false
u16 val: 3 30583
u16 pd: 3 30583 true true false
i16 val: 3 30583
i16 pd: 3 30583 true true false
===*/

function test() {
    var buf = new ArrayBuffer(8);
    var u8 = new Uint8Array(buf);
    var u16 = new Uint16Array(buf);
    var i16 = new Int16Array(buf);

    // init values are endian neutral for u16/i16 views
    u8[0] = 0xff; u8[1] = 0xff;
    u8[2] = 0x80; u8[3] = 0x80;
    u8[4] = 0x11; u8[5] = 0x11;
    u8[6] = 0x77; u8[7] = 0x77;

    [ 'length', 'byteLength', 'byteOffset', 'BYTES_PER_ELEMENT',
      '0', '1', '2', '3' ].forEach(function (propname) {
        print('u16 val:', propname, u16[propname]);
        pd = Object.getOwnPropertyDescriptor(u16, propname);
        print('u16 pd:', propname, pd.value, pd.writable, pd.enumerable, pd.configurable);

        print('i16 val:', propname, i16[propname]);
        pd = Object.getOwnPropertyDescriptor(u16, propname);
        print('i16 pd:', propname, pd.value, pd.writable, pd.enumerable, pd.configurable);
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
