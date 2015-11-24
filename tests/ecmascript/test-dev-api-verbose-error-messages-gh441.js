/*
 *  Fragile error test for (some) API error messages
 */

/*===
TypeError: string required, found none (stack index 0)
TypeError: string required, found undefined (stack index 0)
TypeError: string required, found null (stack index 0)
TypeError: string required, found true (stack index 0)
TypeError: string required, found false (stack index 0)
TypeError: string required, found 123 (stack index 0)
TypeError: string required, found [object Object] (stack index 0)
TypeError: string required, found [object Array] (stack index 0)
TypeError: string required, found [object Function] (stack index 0)
TypeError: string required, found (PTR) (stack index 0)
TypeError: string required, found [buffer:4] (stack index 0)
TypeError: string required, found [object Pointer] (stack index 0)
TypeError: string required, found [object Buffer] (stack index 0)
TypeError: object required, found '' (stack index 0)
TypeError: object required, found 'foo' (stack index 0)
TypeError: object required, found 'foo<4660>' (stack index 0)
TypeError: object required, found 'foobarfoobar\x00foobarfoobar<51966>foobar...' (stack index 0)
TypeError: RegExp required, found 123 (stack index -1)
===*/

function test() {
    function cleanPrint(v) {
        print(String(v).replace(/[^\u0020-\u007e]/g, function (v) { return '<' + v.charCodeAt(0) + '>'; })
                       .replace(/\(0x.*?\)/g, '(PTR)'));
    }

    // Duktape.enc() requires first arg to be string
    [ 'NONE', undefined, null, true, false, 123,
      { foo: 'bar' }, [ 'foo', 'bar' ], function nop() {},
      Duktape.Pointer('dummy'), Duktape.Buffer(4),
      new Duktape.Pointer('dummy'), new Duktape.Buffer(4) ].forEach(function (v) {
        try {
            if (v === 'NONE') {
                print(Duktape.enc());
            } else {
                print(Duktape.enc(v));
            }
        } catch (e) {
            cleanPrint(e);
        }
    });

    // Duktape.fin() requires first arg to be object
    [ '', 'foo', 'foo\u1234', 'foobarfoobar\u0000foobarfoobar\ucafefoobarfoobar\u4321quux' ].forEach(function (v) {
        try {
            print(Duktape.fin(v));
        } catch (e) {
            cleanPrint(e);
        }
    });

    // RegExp uses a "require with class" internal helper
    try {
        RegExp.prototype.exec.call(123);
    } catch (e) {
        cleanPrint(e);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
