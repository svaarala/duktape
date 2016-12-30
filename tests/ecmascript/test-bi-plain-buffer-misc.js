/*@include util-buffer.js@*/

/*===
misc test
0
4 97
4 97
16 [object Uint32Array]
97 97
object false
97 2004318071
TypeError
read using [object Uint8Array] key
read using [Overridden] key
/[object Uint8Array]/
true
true
false
true
false
Error
Error: fake message
===*/

function miscTest() {
    var pb;
    var ab;
    var t;
    var i;

    function resetValues() {
        pb = createPlainBuffer('abcdefghijklmnop');
    }

    // ArrayBuffer constructor no longer handles a plain buffer specially
    // as in Duktape 1.x (which created a Node.js Buffer with the same
    // underlying plain buffer)  Instead, the argument is ToNumber() coerced
    // ultimately resulting in zero, and the result is a zero length ArrayBuffer.
    t = new ArrayBuffer(createPlainBuffer(4));
    print(t.byteLength);

    // Similarly, Node.js Buffer constructor no longer special cases plain
    // buffer.  Instead, a plain buffer is treated like Uint8Array or any
    // other object: its .length is read, and index properties are coerced
    // to form a fresh buffer with matching .length.
    pb = createPlainBuffer(4);
    pb[0] = 0x61; pb[1] = 0x62; pb[2] = 0x63; pb[3] = 0x64;
    t = new Buffer(pb);
    print(t.length, t[0]);
    pb[0] = 0x69;  // demonstrate independent backing buffer
    print(t.length, t[0]);

    // Typed array constructor interprets a plain buffer as an initializer,
    // just like an Uint8Array object.  A new underlying buffer is created.
    pb = createPlainBuffer('abcdefghijklmnop');
    t = new Uint32Array(pb);
    print(t.length, t);
    print(pb[0], t[0]);
    print(typeof t.buffer, t.buffer === pb);
    t[0] = 0x77777777;  // endian neutral; demonstrate separate underlying buffer
    print(pb[0], t[0]);

    // DataView now rejects a plain buffer; ES2015 DataView only accepts an
    // ArrayBuffer argument.
    resetValues();
    try {
        t = new DataView(pb);
    } catch (e) {
        print(e.name);
    }

    // When a plain buffer (or an ArrayBuffer) is used as a property key,
    // they get coerced to a string first, usually '[object Uint8Array]'.
    resetValues();
    var obj = {
        '[object Uint8Array]': 'read using [object Uint8Array] key',
        '[Overridden]': 'read using [Overridden] key'
    };
    print(obj[pb]);
    Uint8Array.prototype.toString = function () { return '[Overridden]'; };
    print(obj[pb]);
    delete Uint8Array.prototype.toString;

    // Create a RegExp whose pattern is a plain buffer.  The buffer argument
    // gets string coerced, usually to '[object Uint8Array]' which is a
    // character class match.
    resetValues();
    var re = new RegExp(pb);
    print(re);
    print(re.test('ABBA'));   // 'A' matches
    print(re.test('xyzzy'));  // 'y' matches
    print(re.test('q'));  // no match

    // Plain buffer as a RegExp is string coerced, usually to '[object Uint8Array]'.
    resetValues();
    var re = /Uint8Array/;
    print(re.test(pb));
    Uint8Array.prototype.toString = function () { return 'something else'; };
    print(re.test(pb));
    delete Uint8Array.prototype.toString;

    // Error.prototype.toString(), has lightfunc handling and also plain buffer
    // handling.  Not much of a practical difference.
    resetValues();
    print(Error.prototype.toString.call(pb));
    Uint8Array.prototype.message = 'fake message';
    print(Error.prototype.toString.call(pb));
    delete Uint8Array.prototype.message;
}
try {
    print('misc test');
    miscTest();
} catch (e) {
    print(e.stack || e);
}
