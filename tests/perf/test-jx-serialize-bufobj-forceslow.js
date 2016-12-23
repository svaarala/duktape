if (typeof print !== 'function') { print = console.log; }

function build() {
    var obj = {};
    var ab = new Uint8Array(32);
    var vw = new Uint32Array(ab, 4, 3);
    var i;
    for (i = 0; i < ab.length; i++) {
      ab[i] = i;
    }

    return {
        foo: [ ab, ab, ab, ab, ab, ab, ab, ab, ab, ab, ab, ab, ab, ab, ab, ab ],
        bar: [ vw, vw, vw, vw, vw, vw, vw, vw, vw, vw, vw, vw, vw, vw, vw, vw ],
        toJSON: 123  // forces slow path in Duktape 1.5.x, but not called
    };
}

function test() {
    var obj;
    var i;
    var ignore;

    obj = build();
    for (i = 0; i < 3e5; i++) {
        ignore = Duktape.enc('jx', obj);
    }
    //print(ignore);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
