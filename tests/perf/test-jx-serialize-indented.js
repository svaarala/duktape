if (typeof print !== 'function') { print = console.log; }

function build() {
    var obj = {};

    // "fast" leaf values only, this test is just for indentation handling
    obj.key1 = void 0;
    obj.key2 = 1/0;
    obj.key3 = -1/0;
    obj.key4 = 0/0;
    obj.key5 = Duktape.dec('hex', 'deadbeef12345678');
    obj.key6 = Duktape.Pointer('dummy');
    obj.key7 = [ 'foo', 'bar', 'quux', 'baz', 'quuux' ];
    obj.key8 = [ undefined, null, true, 123, {}, {}, {} ];

    return {
        foo: [
            obj
        ],
        bar: {
            quux: {
                baz: obj,
                quuux: {
                    quuuux: obj
                }
            }
        }
    };
}

function test() {
    var obj;
    var i;
    var ignore;

    obj = build();
    for (i = 0; i < 5e5; i++) {
        ignore = Duktape.enc('jx', obj, null, 4);
    }
    //print(ignore);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
