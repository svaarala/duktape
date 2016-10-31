/*
 *  String.prototype.repeat() for an internal string just concatenates the
 *  internal representations.  At the join point between segments this may
 *  generate valid codepoints.  This is not a compliance issue because all
 *  compliant strings are CESU-8 encoded and have no such isses; it's also
 *  not a sandboxing issue because the concatenation process can't create
 *  internal keys unless you already have one at hand.  The prefix of the
 *  result is also from the repeated string so you also can't create a new
 *  prefix byte by repeating something.
 */

/*---
{
    "custom": true
}
---*/

/*===
4
65533 65 66 67
12
|ff414243ff414243ff414243|
65533 65 66 67 65533 65 66 67 65533 65 66 67
4
65533 65 66 65533
undefined
12
|abbe414243ecabbe414243ecabbe414243ec|
65533
65533 65 66 67 51966 65 66 51966 65 66 67 65533
===*/

function test() {
    var str, res;

    function dump(x) {
        var res = [];
        for (var i = 0; i < x.length; i++) {
            res.push(x.charCodeAt(i));
        }
        print(res.join(' '));
    }

    // Internal prefix 0xFF is just repeated, unaffected.
    str = String.fromBuffer(new Uint8Array([ 0xff, 0x41, 0x42, 0x43 ]));
    print(str.length);
    dump(str);
    res = str.repeat(3);
    print(res.length);
    print(Duktape.enc('jx', ArrayBuffer.allocPlain(res)));
    dump(res);

    // Here the string begins and ends with a broken codepoint, but
    // repeating the string joins the broken pieces into a valid
    // codepoint; U+CAFE encodes to EC AB BE.
    // This comes out very oddly when iterated using x.charCodeAt()
    // but there are no guarantees for invalid UTF-8 strings so that's
    // OK for this test.
    str = String.fromBuffer(new Uint8Array([ 0xab, 0xbe, 0x41, 0x42, 0x43, 0xec ]));
    print(str.length);
    print(dump(str));
    res = str.repeat(3);
    print(res.length);
    print(Duktape.enc('jx', ArrayBuffer.allocPlain(res)));
    print(res.charCodeAt(0));
    dump(res);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
