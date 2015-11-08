if (typeof print !== 'function') { print = console.log; }

function test() {
    var buf = Duktape.Buffer(31);
    var ref;
    var i;

    for (i = 0; i < buf.length; i++) {
        buf[i] = i;
    }
    ref = "" + buf;

    for (i = 0; i < 1e5; i++) {
        buf[0] = i;  // vary string hash
        buf[1] = i / 256;

        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);

        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);

        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);

        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);

        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);

        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);

        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);

        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);

        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);

        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
        void ("" + buf);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
