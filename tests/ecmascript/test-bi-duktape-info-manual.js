/*
 *  Duktape.info()
 *
 *  To be executed manually.
 */

/*---
{
    "skip": true
}
---*/

/*===
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
        print(idx, typeof v, String(v), typeof info, Duktape.enc('jx', info));
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
