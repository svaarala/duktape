/*
 *  https://github.com/svaarala/duktape/issues/120
 *
 *  E5.1 requires a ToUint16() coercion.  Duktape 2.x uses ToUint32() by
 *  default to better support non-BMP strings.  Duktape 3.x has WTF-8
 *  support for more convenient C API access to string, and uses the
 *  standard ToUint16() coercion.
 */

/*---
custom: true
---*/

/*===
1
65
"A"
3
65535
48879
26505
"\uffff\ubeef\u6789"
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

test();
