if (typeof print !== 'function') { print = console.log; }

function test() {
    var tmp1 = [];
    var tmp2 = [];
    var i, n, buf;

    print('build');
    buf = (Uint8Array.allocPlain || Duktape.Buffer)(1024 * 1024);
    for (i = 0; i < buf.length; i++) {
        buf[i] = i;
    }

    print('run');
    for (i = 0; i < 20; i++) {
        void JSON.stringify(buf);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
