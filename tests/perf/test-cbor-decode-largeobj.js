if (typeof print !== 'function') { print = console.log; }

function build() {
    var obj = {};

    for (var i = 0; i < 3000; i++) {
        var k = 'key' + i;
        var v;
        switch (i % 4) {
        case 0: v = true; break;
        case 1: v = 123.4; break;
        case 2: v = { foo: 1, bar: 2, quux: 3, baz: 4, quuux: 5 }; break;
        case 3: v = [ 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.12345 ]; break;
        }
        obj[k] = v;
    }
    return obj;
}

function test() {
    var obj;
    var buf;
    var i;

    obj = build();
    buf = CBOR.encode(obj);

    for (i = 0; i < 1e3; i++) {
        void CBOR.decode(buf);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
