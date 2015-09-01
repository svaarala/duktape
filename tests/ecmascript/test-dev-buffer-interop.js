/*
 *  A few buffer creation, coercion and interoperability tests.
 */

/*===
plain buffer test
buffer ABCD
buffer ABCD
object ABCD
buffer buffer buffer
true true true
object object buffer buffer
false
ABCDEFGH
ABCDEFGH
===*/

/* Create buffer values and extract plain buffer inside. */

function plainBufferTest() {
    var a, b, c, d, e, f;

    // plain buffer, Duktape.Buffer(), new Duktape.Buffer()
    a = Duktape.dec('hex', '41424344');
    print(typeof a, a);
    b = Duktape.Buffer(a);
    print(typeof b, b);
    c = new Duktape.Buffer(a);
    print(typeof c, c);
    d = a.valueOf();  // valueOf returns the underlying buffer, no copy
    e = b.valueOf();
    f = c.valueOf();
    print(typeof d, typeof e, typeof f);
    print(d === e, d === e, e === f);

    // Node.js Buffer constructor accepts an arbitrary buffer object input
    // but makes a copy (doesn't "embed" the underlying input buffer in the
    // result).

    a = new Buffer('ABCDEFGH');
    b = new Buffer(a);
    c = Duktape.Buffer(a);   // get underlying buffer
    d = Duktape.Buffer(b);
    print(typeof a, typeof b, typeof c, typeof d);
    print(c === d);  // not the same
    print(a);
    print(b);

    // ArrayBuffer constructor doesn't accept another buffer so there's
    // no buffer copy/embed behavior to test at the moment.

    // XXX: buffer coercion / interoperability is under development so
    // this test will need to be updated.
}

try {
    print('plain buffer test');
    plainBufferTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Node.js Buffer constructor interop
{type:"Buffer",data:[102,111,111]}
true 97 97
{type:"Buffer",data:[102,111,111]}
false 97 102
{type:"Buffer",data:[1,255]}
false 2 1
{type:"Buffer",data:[239,123]}
{type:"Buffer",data:[3]}
===*/

/* Creating a Node.js Buffer from other buffer values. */

function nodejsBufferConstructorTest() {
    var b;
    var t;

    // A plain buffer is accepted as Node.js Buffer() input, and is used as
    // the internal buffer value *without making a copy*.  This behavior is
    // (of course) custom, and matches Duktape.Buffer() behavior.

    t = Duktape.Buffer('foo');  // plain
    b = new Buffer(t);
    print(Duktape.enc('jx', b));
    b[0] = 0x61;  // 'a'
    print(b[0] === t[0], b[0], t[0]);

    // A Duktape.Buffer is accepted as an input, interpreted as an Array-like
    // input.  A copy (not slice) is made.

    t = new Duktape.Buffer('foo');
    b = new Buffer(t);
    print(Duktape.enc('jx', b));
    b[0] = 0x61;  // 'a'
    print(b[0] === t[0], b[0], t[0]);

    // An ArrayBuffer can be used as an input to create a Node.js Buffer; it
    // is interpreted as an Array-like input.  A copy (not slice) is made.

    t = new ArrayBuffer(2);
    t[0] = 0x01;
    t[1] = 0xff;
    b = new Buffer(t);
    print(Duktape.enc('jx', b));
    b[0] = 0x02;
    print(b[0] === t[0], b[0], t[0]);

    // A TypedArray can be used as an input to create a Node.js Buffer; it
    // is interpreted as an Array-like input.

    t = new Uint32Array([0xdeadbeef, 123])
    b = new Buffer(t);
    print(Duktape.enc('jx', b));

    // A TypedArray gets coerced through an indexed read and a Buffer write.
    // Here the input is a Math.PI in a Float32 typedarray and coerces to 3.

    t = new Float32Array([Math.PI])
    b = new Buffer(t);
    print(Duktape.enc('jx', b));
}

try {
    print('Node.js Buffer constructor interop');
    nodejsBufferConstructorTest();
} catch (e) {
    print(e.stack || e);
}

/*===
Node.js Buffer.concat interop
foobarABCbcde
===*/

function nodejsConcatTest() {
    var res;
    var b1, b2, b3, b4;

    // Concat accepts all view/buffer types.  View/slice offsets are
    // respected, e.g. b4 first byte is skipped below.

    b1 = new Buffer('foo');
    b2 = new Duktape.Buffer('bar');
    b3 = new ArrayBuffer(3); b3[0] = 0x41; b3[1] = 0x42; b3[2] = 0x43;
    b4 = new Uint8Array([ 0x61, 0x62, 0x63, 0x64, 0x65 ]).subarray(1);

    res = Buffer.concat([ b1, b2, b3, b4 ]);
    print(res);
}

try {
    print('Node.js Buffer.concat interop');
    nodejsConcatTest();
} catch (e) {
    print(e.stack || e);
}

/*===
ArrayBuffer constructor interop
|666f6f|
true 97 97
===*/

function arrayBufferConstructorTest() {
    var b;
    var t;

    // A plain buffer is accepted as ArrayBuffer() input, and is used as
    // the internal buffer value *without making a copy*.  This behavior
    // is (of course) custom, and matches Duktape.Buffer() behavior.

    t = Duktape.Buffer('foo');  // plain
    b = new ArrayBuffer(t);
    print(Duktape.enc('jx', b));
    b[0] = 0x61;  // 'a'
    print(b[0] === t[0], b[0], t[0]);

    // ArrayBuffer doesn't accept other buffer objects as input at the
    // moment though; such values are integer coerced and used as a length.
}

try {
    print('ArrayBuffer constructor interop');
    arrayBufferConstructorTest();
} catch (e) {
    print(e.stack || e);
}
