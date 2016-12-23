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

/*@include util-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
4
65533 65 66 67
12
|fe414243fe414243fe414243|
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

    // Internal prefix 0xFE is just repeated, unaffected.
    str = bufferToStringRaw(new Uint8Array([ 0xfe, 0x41, 0x42, 0x43 ]));
    print(str.length);
    dump(str);
    res = str.repeat(3);
    print(res.length);
    print(Duktape.enc('jx', Uint8Array.allocPlain(res)));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
