if (typeof print !== 'function') { print = console.log; }

function test() {
    var buf = (Uint8Array.allocPlain || Duktape.Buffer)(31);
    var i, j;
    var arr;
    var bufferToString = String.fromBufferRaw || String;

    for (i = 0; i < buf.length; i++) {
        buf[i] = i;
    }

    for (i = 0; i < 1e4; i++) {
        arr = [];
        for (j = 0; j < 1e3; j++) {
            // make buffer value unique
            buf[0] = j;
            buf[1] = j >> 8;
            buf[2] = j >> 16;
            arr[j] = bufferToString(buf);
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
