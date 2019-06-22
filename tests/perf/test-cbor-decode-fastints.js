if (typeof print !== 'function') { print = console.log; }

function build() {
    var obj = [];

    for (var i = 0; i < 1000; i++) {
        obj.push(123);
        obj.push(12);
        obj.push(123);
        obj.push(12);
        obj.push(123);
        obj.push(12);
        obj.push(-123);
        obj.push(-12);
        obj.push(-123);
        obj.push(-12);
        obj.push(-123);
        obj.push(-12);
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
