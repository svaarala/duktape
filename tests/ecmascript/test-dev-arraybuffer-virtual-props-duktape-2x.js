/*
 *  Specific test for virtual properties removed from ArrayBuffer in
 *  Duktape 2.x.
 */

/*===
length false false undefined undefined
byteLength true false number 10
byteOffset false false undefined undefined
BYTES_PER_ELEMENT false false undefined undefined
0 false false undefined undefined
9 false false undefined undefined
10 false false undefined undefined
0 16 undefined
1 17 undefined
2 18 undefined
3 19 undefined
4 20 undefined
5 21 123
6 22 foo
7 23 undefined
8 24 undefined
9 25 undefined
bar
===*/

function test() {
    var buf = new ArrayBuffer(10);
    var u8;
    var pd;
    var i;

    // In Duktape 2.0 .byteLength is a virtual own property which is not
    // ES2015 compliant, where it is required to be an inherited accessor.

    [ 'length', 'byteLength', 'byteOffset', 'BYTES_PER_ELEMENT',
      '0', '9', '10' ].forEach(function (v) {
        print(v, v in buf, buf.hasOwnProperty(v), typeof buf[v], buf[v]);
    });

    // Because index properties have no special behavior, they can be written
    // to normally without affecting the buffer data.

    buf = new ArrayBuffer(10);
    u8 = new Uint8Array(buf);
    for (i = 0; i < u8.length; i++) {
        u8[i] = 0x10 + i;
    }
    buf[5] = 123;
    buf[6] = 'foo';
    buf[100] = 'bar';
    for (i = 0; i < u8.length; i++) {
        print(i, u8[i], buf[i]);
    }
    print(buf[100]);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
