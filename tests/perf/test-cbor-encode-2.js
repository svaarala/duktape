if (typeof print !== 'function') { print = console.log; }

function test() {
    var msg = {
        jsonrpc: '2.0',
        method: 'FooBar',
        params: {
            arg1: 'xxxxxyyyyy',
            arg2: 'xxxxxyyyyyzzzzzwwwww',
            arg3: 'xxxxxyyyyy',
            arg4: 'xxxxxyyyyyzzzzzwwwww',
            arg5: 'xxxxxyyyyy',
            arg6: 'xxxxxyyyyyzzzzzwwwww',
            arg7: 'xxxxxyyyyy',
            arg8: 'xxxxxyyyyyzzzzzwwwww',
            arg9: 'xxxxxyyyyy',
            arg10: 'xxxxxyyyyyzzzzzwwwww',
            arg11: [ 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0 ]
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
