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

    // print(Duktape.enc('hex', CBOR.encode(msg)));
    // a3676a736f6e72706363322e30666d6574686f6466466f6f42617266706172616d73a363666f6f187b6362617283f5f4fb405edd2f1a9fbe776471757578a26362617af5657175757578f4

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
