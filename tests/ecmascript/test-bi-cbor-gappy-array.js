/*===
,,,foo,,,,,bar,
10
0: false
1: false
2: false
3: true
4: false
5: false
6: false
7: false
8: true
9: false
10: false
11: false
8af7f7f763666f6ff7f7f7f763626172f7
,,,foo,,,,,bar,
10
0: true
1: true
2: true
3: true
4: true
5: true
6: true
7: true
8: true
9: true
10: false
11: false
===*/

function test() {
    var i;

    var arr = new Array(10);
    arr[3] = 'foo';
    arr[8] = 'bar';

    print(String(arr));
    print(arr.length);
    for (i = 0; i < 12; i++) {
        print(i + ': ' + (i in arr));
    }

    // CBOR does not distinguish between an 'undefined' and a missing
    // value (gap).  So the gaps get encoded as 'undefined' up to the
    // array length (10).  When decoded, the arrays comes out with length
    // 10 and gaps replaced with 'undefined'.

    var enc = CBOR.encode(arr);
    print(Duktape.enc('hex', enc));
    arr = CBOR.decode(enc);

    print(String(arr));
    print(arr.length);
    for (i = 0; i < 12; i++) {
        print(i + ': ' + (i in arr));
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
