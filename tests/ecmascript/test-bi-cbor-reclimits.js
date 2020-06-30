/*---
{
    "custom": true
}
---*/

/*===
encode 999 ok
encode 1000 ok
encode 1001 RangeError: encode recursion limit
encode 10000 RangeError: encode recursion limit
decode 999 ok
decode 1000 ok
decode 1001 RangeError: decode recursion limit
decode 10000 RangeError: decode recursion limit
done
===*/

function mkObj(n) {
    var res = 1;
    for (var i = 0; i < n; i++) {
        res = { foo: 123, ref: res };
    }
    return res;
}

function mkBuf(n) {
    var arr = [ 0x01 ];  // uint 1
    for (var i = 0; i < n; i++) {
        arr = [ 0xa1, 0x63, 0x66, 0x6f, 0x6f ].concat(arr);  // { foo: x }
    }
    return new Uint8Array(arr);
}

[ 999, 1000, 1001, 10000 ].forEach(function (n) {
    try {
        void CBOR.encode(mkObj(n));
        print('encode', n, 'ok');
    } catch (e) {
        print('encode', n, String(e));
    }
});

[ 999, 1000, 1001, 10000 ].forEach(function (n) {
    try {
        void CBOR.decode(mkBuf(n));
        print('decode', n, 'ok');
    } catch (e) {
        print('decode', n, String(e));
    }
});

print('done');
