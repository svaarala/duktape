/*
 *  Simulate creation and JSON serialization of a fake JSONRPC message.
 */

function test() {
    var i;
    var msg;
    var bytes;
    var ign;

    bytes = Duktape.Buffer(4096);
    for (i = 0; i < bytes.length; i++) {
        bytes[i] = i;
    }
    for (i = 0; i < 1e5; i++) {
        msg = {
            jsonrpc: '2.0',
            id: 'dummy',
            method: 'FakeMethod',
            params: {
                client: 'dummy client',
                timestamp: Date.now(),
                data: Duktape.enc('base64', bytes)
            }
        };
        ign = JSON.stringify(msg);
    }
    print(ign);
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
