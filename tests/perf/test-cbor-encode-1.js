if (typeof print !== 'function') { print = console.log; }

function test() {
    var msg = {
        jsonrpc: '2.0',
        method: 'FooBar',
        params: {
            foo: 123,
            bar: [ true, false, 123.456 ],
            quux: {
                baz: true,
                quuux: false
            }
        }
    };

    for (var i = 0; i < 1e5; i++) {
        void CBOR.encode(msg);
        void CBOR.encode(msg);
        void CBOR.encode(msg);
        void CBOR.encode(msg);
        void CBOR.encode(msg);
        void CBOR.encode(msg);
        void CBOR.encode(msg);
        void CBOR.encode(msg);
        void CBOR.encode(msg);
        void CBOR.encode(msg);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
