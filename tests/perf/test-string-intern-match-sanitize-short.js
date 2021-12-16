if (typeof print !== 'function') { print = console.log; }

function test() {
    var buf = (Uint8Array.allocPlain || Duktape.Buffer)(31);
    var ref;
    var i;
    var bufferToString = String.fromBufferRaw || String;

    for (i = 0; i < buf.length; i++) {
        buf[i] = i;
    }
    ref = bufferToString(buf);

    for (i = 0; i < 1e5; i++) {
        buf[0] = i;  // vary string hash
        buf[1] = i / 256;

        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);

        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);

        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);

        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);

        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);

        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);

        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);

        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);

        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);

        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
        void bufferToString(buf);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
