/*
 *  https://github.com/svaarala/duktape/issues/120
 *
 *  E5.1 requires a ToUint16() coercion but Duktape uses ToUint32() by
 *  default to better support non-BMP strings.
 */

/*---
{
    "custom": true
}
---*/

/*===
1
65601
"\U00010041"
3
4294967295
3735928559
591751049
"\Uffffffff\Udeadbeef\U23456789"
===*/

function test() {
    var s = String.fromCharCode(0x10041);
    print(s.length);
    print(s.charCodeAt(0));
    print(Duktape.enc('jx', s));

    var s = String.fromCharCode(0xffffffff, 0xdeadbeef, 0x123456789 /* over 32-bit */);
    print(s.length);
    print(s.charCodeAt(0));
    print(s.charCodeAt(1));
    print(s.charCodeAt(2));
    print(Duktape.enc('jx', s));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
