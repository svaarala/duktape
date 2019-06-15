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

    // print(Duktape.enc('hex', CBOR.encode(msg)));
    // a3676a736f6e72706363322e30666d6574686f6466466f6f42617266706172616d73ab64617267316a78787878787979797979646172673274787878787879797979797a7a7a7a7a777777777764617267336a78787878787979797979646172673474787878787879797979797a7a7a7a7a777777777764617267356a78787878787979797979646172673674787878787879797979797a7a7a7a7a777777777764617267376a78787878787979797979646172673874787878787879797979797a7a7a7a7a777777777764617267396a7878787878797979797965617267313074787878787879797979797a7a7a7a7a77777777776561726731318b01fb3ff199999999999afb3ff3333333333333fb3ff4cccccccccccdfb3ff6666666666666f93e00fb3ff999999999999afb3ffb333333333333fb3ffccccccccccccdfb3ffe66666666666602


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
