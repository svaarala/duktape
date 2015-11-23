/*
 *  Fragile testcase for testing property error messages.
 */

/*---
{
    "custom": true
}
---*/

/*===
TypeError: cannot read property 'foo' of null
TypeError: cannot write property 'foo' of undefined
TypeError: cannot delete property 'foo' of null
"TypeError: cannot write property undefined of null"
"TypeError: cannot write property null of null"
"TypeError: cannot write property true of null"
"TypeError: cannot write property false of null"
"TypeError: cannot write property 123 of null"
"TypeError: cannot write property 123 of null"
"TypeError: cannot write property 0 of null"
"TypeError: cannot write property 0 of null"
"TypeError: cannot write property NaN of null"
"TypeError: cannot write property Infinity of null"
"TypeError: cannot write property -Infinity of null"
"TypeError: cannot write property '' of null"
"TypeError: cannot write property 'foo' of null"
"TypeError: cannot write property '12345678901234567890123456789012' of null"
"TypeError: cannot write property '12345678901234567890123456789012...' of null"
"TypeError: cannot write property '1234\\x00678901234567890123456789012' of null"
"TypeError: cannot write property '\u1234foo' of null"
"TypeError: cannot write property '\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345' of null"
"TypeError: cannot write property '\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345...' of null"
"TypeError: cannot write property 'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx...' of null"
"TypeError: cannot write property '\\x00\\x1f\\x27\\x5c\\x7f' of null"
"TypeError: cannot write property [object Array] of null"
"TypeError: cannot write property [object Object] of null"
"TypeError: cannot write property [object Function] of null"
"TypeError: cannot write property (PTR) of null"
"TypeError: cannot write property [object Pointer] of null"
"TypeError: cannot write property [buffer:5] of null"
"TypeError: cannot write property [object Buffer] of null"
===*/

function test() {
    var tmp;

    // Basic cases
    try {
        print(null.foo);
    } catch (e) {
        print(e);
    }
    try {
        undefined.foo = 123;
    } catch (e) {
        print(e);
    }
    try {
        delete null.foo;
    } catch (e) {
        print(e);
    }

    // The same internal summarization API call is used for summarizing
    // object and key.  The summary must be human readable, limited length,
    // and side effect free.  Test a few basic cases.

    var longstring = 'x'
    for (var i = 0; i < 20; i++) { longstring += longstring; }
    [
        undefined, null, true, false, 123, 123.0, -0, +0, 0/0, 1/0, -1/0,
        '', 'foo',
        '12345678901234567890123456789012',  // 32 chars is currently shown as is
        '12345678901234567890123456789012x',
        '1234\u0000678901234567890123456789012',  // 32 chars, internal NUL
        '\u1234foo',  // unicode
        '\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345',  // 32 unicode chars
        '\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\u1234\u2345\ucafe',  // 33 unicode chars
        longstring,
	'\u0000\u001f\u0027\u005c\u007f',  // some escaped chars
        [ 1, 2, 3 ],
        { foo:'bar' },
        function test() {},
        Duktape.Pointer('dummy'),
        new Duktape.Pointer('dummy'),
        Duktape.Buffer('dummy'),
        new Duktape.Buffer('dummy'),
    ].forEach(function (v) {
        try {
            null[v] = 123;
        } catch (e) {
            tmp = Duktape.enc('jx', String(e));  // JX encode to get ASCII
            tmp = tmp.replace(/\(0x[0-9a-fA-F]+\)/, '(PTR)');  // replace pointer for expect string
            print(tmp);
        }
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
