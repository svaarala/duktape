if (typeof print !== 'function') { print = console.log; }

function build() {
    var obj = {};

    // "fast" leaf values only, this test is just for indentation handling
    obj.key1 = 'foo';
    obj.key2 = 'bar';
    obj.key3 = 'quux';
    obj.key4 = 'baz';
    obj.key5 = 'quuux';
    obj.key6 = [ 'foo', 'bar', 'quux', 'baz', 'quuux' ];
    obj.key7 = [ undefined, null, true, 123, {}, {}, {} ];

    for (var i = 0; i < 100; i++) {
        obj = { foo: obj };
    }

    return obj;
}

function test() {
    var obj;
    var i;
    var ignore;

    obj = build();
    for (i = 0; i < 1e4; i++) {
        ignore = JSON.stringify(obj, null, 4);
    }
    //print(ignore);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
