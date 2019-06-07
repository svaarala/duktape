if (typeof print !== 'function') { print = console.log; }

function build() {
    var obj = {};

    obj.key1 = 'foo';
    obj.key2 = 'bar';
    obj.key3 = 'quux';
    obj.key4 = 'baz';
    obj.key5 = 'quuux';
    obj.key6 = [ 'foo', 'bar', 'quux', 'baz', 'quuux' ];
    obj.key7 = [ undefined, null, true, 123.456, 1e200, {}, {}, {} ];

    return obj;
}

function test() {
    var obj;
    var i;
    var ignore;

    obj = build();
    for (i = 0; i < 4e5; i++) {
        ignore = CBOR.encode(obj);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
