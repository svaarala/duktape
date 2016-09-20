function test() {
    var i;
    var t;

    for (i = 0; i < 2e5; i++) {
        t = { foo: 1, bar: 2, quux: 3, baz: 4, quuux: 5, quuuux: 6, quuuuux: 7, quuuuuux: 8, quuuuuuux: 9, quuuuuuuux: 10 };
        t = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
        t = { foo: 1, bar: 2, quux: 3, baz: 4, quuux: 5, quuuux: 6, quuuuux: 7, quuuuuux: 8, quuuuuuux: 9, quuuuuuuux: 10 };
        t = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
        t = { foo: 1, bar: 2, quux: 3, baz: 4, quuux: 5, quuuux: 6, quuuuux: 7, quuuuuux: 8, quuuuuuux: 9, quuuuuuuux: 10 };
        t = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
        t = { foo: 1, bar: 2, quux: 3, baz: 4, quuux: 5, quuuux: 6, quuuuux: 7, quuuuuux: 8, quuuuuuux: 9, quuuuuuuux: 10 };
        t = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
        t = { foo: 1, bar: 2, quux: 3, baz: 4, quuux: 5, quuuux: 6, quuuuux: 7, quuuuuux: 8, quuuuuuux: 9, quuuuuuuux: 10 };
        t = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
        t = { foo: 1, bar: 2, quux: 3, baz: 4, quuux: 5, quuuux: 6, quuuuux: 7, quuuuuux: 8, quuuuuuux: 9, quuuuuuuux: 10 };
        t = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
        t = { foo: 1, bar: 2, quux: 3, baz: 4, quuux: 5, quuuux: 6, quuuuux: 7, quuuuuux: 8, quuuuuuux: 9, quuuuuuuux: 10 };
        t = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
        t = { foo: 1, bar: 2, quux: 3, baz: 4, quuux: 5, quuuux: 6, quuuuux: 7, quuuuuux: 8, quuuuuuux: 9, quuuuuuuux: 10 };
        t = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
        t = { foo: 1, bar: 2, quux: 3, baz: 4, quuux: 5, quuuux: 6, quuuuux: 7, quuuuuux: 8, quuuuuuux: 9, quuuuuuuux: 10 };
        t = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
        t = { foo: 1, bar: 2, quux: 3, baz: 4, quuux: 5, quuuux: 6, quuuuux: 7, quuuuuux: 8, quuuuuuux: 9, quuuuuuuux: 10 };
        t = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
