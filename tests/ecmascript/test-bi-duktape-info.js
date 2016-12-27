/*
 *  Duktape.info()
 *
 *  While the results are version specific, test for (some) currently
 *  provided results.
 */

/*===
0 undefined object 1
1 object object 2
2 boolean object 3
3 boolean object 3
4 number object 4
5 number object 4
6 string object 5
7 string object 5
8 object object 6
9 object object 6
10 function object 6
11 function object 6
12 object object 6
13 object object 6
14 object object 7
15 pointer object 8
16 object object 6
17 object object 6
===*/

function test() {
    // Test public type tags.
    [
        void 0, null, true, false, 123, 123.1,
        '', 'foo', { foo: 123 }, [ 1, 2, 3 ],
        Math.cos, function foo() {},
        new ArrayBuffer(10),
        new Uint32Array(4),
        Uint8Array.allocPlain(10),
        Duktape.Pointer('foo'), new Duktape.Pointer('foo'),
        new Duktape.Thread(function () {})
    ].forEach(function (v, idx) {
        var info = Duktape.info(v);
        print(idx, typeof v, typeof info, info.type);
    });

    // XXX
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
