/*@include util-buffer.js@*/

/*===
Duktape methods
- fin
TypeError
TypeError
- enc
|6162636465666768696a6b6c6d6e6f70|
{"_buf":"6162636465666768696a6b6c6d6e6f70"}
6162636465666768696a6b6c6d6e6f70
YWJjZGVmZ2hpamtsbW5vcA==
- dec
foo
true
string 61626364
abcd
true
- compact
[object Uint8Array]
true
===*/

function duktapeMethodTest() {
    var pb, t;

    pb = createPlainBuffer('abcdefghijklmnop');

    // info(): not relevant
    // act(): not relevant
    // gc(): not relevant

    // fin() fails with TypeError, cannot get/set a finalizer on a plain buffer
    print('- fin');
    try {
        print(Duktape.fin(pb));
    } catch (e) {
        print(e.name);
    }
    try {
        print(Duktape.fin(pb, function dummy() {}));
        print(Duktape.fin(pb));
    } catch (e) {
        print(e.name);
    }

    print('- enc');
    print(Duktape.enc('jx', pb));
    print(Duktape.enc('jc', pb));
    print(Duktape.enc('hex', pb));
    print(Duktape.enc('base64', pb));

    // Duktape.dec() interprets a plain buffer like an input string.
    print('- dec');
    t = '666f6f';
    pb = createPlainBuffer(6);
    pb[0] = t.charCodeAt(0);
    pb[1] = t.charCodeAt(1);
    pb[2] = t.charCodeAt(2);
    pb[3] = t.charCodeAt(3);
    pb[4] = t.charCodeAt(4);
    pb[5] = t.charCodeAt(5);
    print(bufferToStringRaw(Duktape.dec('hex', pb)));  // hex decode '666f6f' to buffer containing 'foo'

    // Duktape.dec() outputs a plain buffer.
    pb = createPlainBuffer(4);
    pb[0] = 0x61;
    pb[1] = 0x62;
    pb[2] = 0x63;
    pb[3] = 0x64;
    print(isPlainBuffer(pb));
    t = Duktape.enc('hex', pb);
    print(typeof t, t);
    t = Duktape.dec('hex', t);
    print(bufferToStringRaw(t));
    print(isPlainBuffer(t));

    // compact() is a no-op and returns input value
    print('- compact');
    print(String(Duktape.compact(pb)));
    print(isPlainBuffer(Duktape.compact(pb)));
}

try {
    print('Duktape methods');
    duktapeMethodTest();
} catch (e) {
    print(e.stack || e);
}
