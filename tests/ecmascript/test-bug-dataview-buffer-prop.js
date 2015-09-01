/*
 *  Limitation in DataView constructor handling of result value .buffer
 *  property.
 *
 *  Non-ArrayBuffer arguments are accepted which is intentional to maximizing
 *  buffer type interoperability, but .buffer should really be set only if the
 *  property value is an ArrayBuffer.
 *
 *  Note that this is not an issue for TypedArray constructors which treat
 *  buffers as an "array-like" argument and copy the indexed elements.
 */

/*---
{
    "custom": true
}
---*/

/*===
[object ArrayBuffer]
true
[object ArrayBuffer]
true
[object ArrayBuffer]
true
[object ArrayBuffer]
false
[object ArrayBuffer]
false
===*/

function dataViewTest() {
    var v;
    var buf = new ArrayBuffer(16);

    // ArrayBuffer argument: this is the ordinary case in Khronos/ES6 spec
    v = new DataView(buf);
    print(Object.prototype.toString.call(v.buffer));
    print(v.buffer === buf);

    // DataView argument: non-standard, but best behavior would probably be
    // to copy the argument's .buffer property.
    v = new DataView(new DataView(buf));
    print(Object.prototype.toString.call(v.buffer));
    print(v.buffer === buf);

    // TypedArray argument: non-standard, best behavior also probably to copy
    // argument's .buffer property.
    v = new DataView(new Uint16Array(buf));
    print(Object.prototype.toString.call(v.buffer));
    print(v.buffer === buf);

    // Duktape.Buffer argument: non-standard, best behavior probably not to
    // set a .buffer argument?  Or to create a fresh ArrayBuffer for the
    // .buffer property?
    // XXX: to be decided
    v = new DataView(new Duktape.Buffer('dummy'));
    print(Object.prototype.toString.call(v.buffer));
    print(v.buffer === buf);

    // Node.js Buffer argument: non-standard, best behavior probably not to
    // set a .buffer argument?  Or to create a fresh ArrayBuffer for the
    // .buffer property?
    // XXX: to be decided
    v = new DataView(new Buffer('foobar'));
    print(Object.prototype.toString.call(v.buffer));
    print(v.buffer === buf);
}

try {
    dataViewTest();
} catch (e) {
    print(e.stack || e);
}
