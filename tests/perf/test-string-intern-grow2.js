if (typeof print !== 'function') { print = console.log; }

function test() {
    var buf = Duktape.Buffer(2048);
    var i, j;
    var arr;

    for (i = 0; i < buf.length; i++) {
        buf[i] = i;
    }

    for (i = 0; i < 1e3; i++) {
        arr = [];
        for (j = 0; j < 1e3; j++) {
            // make buffer value unique; position counter at end of string
            // (makes a difference for Lua-like reverse hash direction).
            buf[2047] = j;
            buf[2046] = j >> 8;
            buf[2045] = j >> 16;
            arr[j] = "" + buf;
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
