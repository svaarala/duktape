function test() {
    var msg = [];
    while (msg.length < 10000) {
        msg[msg.length] = 1.5;
    }

    // print(Duktape.enc('hex', CBOR.encode(msg)));

    for (var i = 0; i < 1e3; i++) {
        void CBOR.encode(msg);
    }
}

test();
