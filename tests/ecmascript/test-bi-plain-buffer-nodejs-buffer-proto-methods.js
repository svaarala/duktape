/*@include util-buffer.js@*/

/*===
Node.js Buffer.prototype methods
- toJSON
{type:"Buffer",data:[97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
{type:"Buffer",data:[97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
- compare
equal
0
0
0
0
not equal
0
-1
1
0
- equals
equal
true
true
true
true
not equal
true
false
false
true
- fill
false
[object Uint8Array]
|6162111111111111116a6b6c6d6e6f70|
- copy, source plain buffer, target Node.js Buffer
{type:"Buffer",data:[97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
16
{type:"Buffer",data:[97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
2
{type:"Buffer",data:[97,98,99,111,112,102,103,104,105,106,107,108,109,110,111,112]}
- copy, source plain buffer, target plain buffer
|0000000000000000000000000000000000000000000000000000000000000000|
16
|6162636465666768696a6b6c6d6e6f7000000000000000000000000000000000|
2
|6162636465666768696a6b6c6d6e6f70000000000000006f7000000000000000|
- slice
object cdefg false
|6162636465666768696a6b6c6d6e6f70|
{type:"Buffer",data:[99,100,101,102,103]}
|6162636465ffee68696a6b6c6d6e6f70|
{type:"Buffer",data:[99,100,101,255,238]}
- write
|6162636465666768696a6b6c6d6e6f70| abcdefghijklmnop
3
|616263464f4f6768696a6b6c6d6e6f70| abcFOOghijklmnop
2
|616263464f4f6742416a6b6c6d6e6f70| abcFOOgBAjklmnop
- readUInt16LE
25442
- writeUInt32LE
|6162636465666768696a6b6c6d6e6f70|
11
|61626364656667efbeadde6c6d6e6f70|
- toString
abcdefghijklmnop
===*/

function nodejsBufferPrototypeMethodTest() {
    var pb, nb, t;

    function resetValues() {
        pb = createPlainBuffer('abcdefghijklmnop');
        nb = new Buffer('abcdefghijklmnop');
    }

    /* While Node.js Buffer.prototype methods are intended for Buffer
     * instances only, they also work for ArrayBuffers and plain buffers
     * in Duktape.
     */

    resetValues();
    print('- toJSON');
    print(Duktape.enc('jx', Buffer.prototype.toJSON.call(pb)));
    print(Duktape.enc('jx', Buffer.prototype.toJSON.call(nb)));

    resetValues();
    print('- compare');
    print('equal');
    print(Buffer.prototype.compare.call(pb, pb));
    print(Buffer.prototype.compare.call(pb, nb));
    print(Buffer.prototype.compare.call(nb, pb));
    print(Buffer.prototype.compare.call(nb, nb));
    print('not equal');
    pb[0] = 100; pb[1] = 100;
    nb[0] = 100; nb[1] = 101;
    print(Buffer.prototype.compare.call(pb, pb));
    print(Buffer.prototype.compare.call(pb, nb));
    print(Buffer.prototype.compare.call(nb, pb));
    print(Buffer.prototype.compare.call(nb, nb));

    resetValues();
    print('- equals');
    print('equal');
    print(Buffer.prototype.equals.call(pb, pb));
    print(Buffer.prototype.equals.call(pb, nb));
    print(Buffer.prototype.equals.call(nb, pb));
    print(Buffer.prototype.equals.call(nb, nb));
    print('not equal');
    pb[0] = 100; pb[1] = 100;
    nb[0] = 100; nb[1] = 101;
    print(Buffer.prototype.equals.call(pb, pb));
    print(Buffer.prototype.equals.call(pb, nb));
    print(Buffer.prototype.equals.call(nb, pb));
    print(Buffer.prototype.equals.call(nb, nb));

    resetValues();
    print('- fill');
    var res = Buffer.prototype.fill.call(pb, 0x11, 2, 9);
    print(res === pb);  // false, promoted to object
    print(Object.prototype.toString.call(res));
    print(Duktape.enc('jx', pb));

    resetValues();
    print('- copy, source plain buffer, target Node.js Buffer');
    print(Duktape.enc('jx', nb));
    print(Buffer.prototype.copy.call(pb, nb));
    print(Duktape.enc('jx', nb));
    print(Buffer.prototype.copy.call(pb, nb, 3 /*targetStart*/, 14 /*sourceStart*/, 16 /*sourceEnd*/));
    print(Duktape.enc('jx', nb));

    resetValues();
    print('- copy, source plain buffer, target plain buffer');
    t = createPlainBuffer(32);
    print(Duktape.enc('jx', t));
    print(Buffer.prototype.copy.call(pb, t));
    print(Duktape.enc('jx', t));
    print(Buffer.prototype.copy.call(pb, t, 23 /*targetStart*/, 14 /*sourceStart*/, 16 /*sourceEnd*/));
    print(Duktape.enc('jx', t));

    // .slice() for a plain buffer returns a Node.js Buffer (an Uint8Array
    // inheriting from Buffer.prototype) because a plain buffer cannot
    // represent an offset/length slice (view) into the argument
    resetValues();
    print('- slice');
    t = Buffer.prototype.slice.call(pb, 2, 7);
    print(typeof t, t, isPlainBuffer(t));
    print(Duktape.enc('jx', pb));
    print(Duktape.enc('jx', t));
    t[3] = 0xff;  // demonstrate same underlying buffer
    pb[6] = 0xee;
    print(Duktape.enc('jx', pb));
    print(Duktape.enc('jx', t));

    resetValues();
    print('- write');
    print(Duktape.enc('jx', pb), bufferToStringRaw(pb));
    print(Buffer.prototype.write.call(pb, 'FOO', 3));
    print(Duktape.enc('jx', pb), bufferToStringRaw(pb));
    print(Buffer.prototype.write.call(pb, 'BARQUUX', 7, 2));
    print(Duktape.enc('jx', pb), bufferToStringRaw(pb));

    // Spot check one read method
    resetValues();
    print('- readUInt16LE');
    print(Buffer.prototype.readUInt16LE.call(pb, 1));  // 62 63 in memory -> 0x6362

    // Spot check one write method
    resetValues();
    print('- writeUInt32LE');
    print(Duktape.enc('jx', pb));
    print(Buffer.prototype.writeUInt32LE.call(pb, 0xdeadbeef, 7));
    print(Duktape.enc('jx', pb));

    resetValues();
    print('- toString');
    print(Buffer.prototype.toString.call(pb));
}

try {
    print('Node.js Buffer.prototype methods');
    nodejsBufferPrototypeMethodTest();
} catch (e) {
    print(e.stack || e);
}
