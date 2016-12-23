/*@include util-buffer.js@*/

/*===
coercion test
false
[object Uint8Array]
[Overridden]
TypeError: coercion to primitive failed
NaN
123
object [object Uint8Array] object
false
object [object Uint8Array] object
object [object ArrayBuffer]
|6162636465666768696a6b6c6d6e6f70|
|6162636465666768696a6b6c6d6e6f70|
|6162556465666768696a6b6c6d6e6f70|
|6162556465666768696a6b6c6d6e6f70|
|6162556465666768696a6b6c6d6e6f70|
===*/

function coercionTest() {
    var pb = createPlainBuffer('abcdefghijklmnop');
    var obj;
    var t;
    var pd;

    // ES5 coercions

    // ToObject() coercion returns a full Uint8Array object, so that
    // Object(plain) !== plain.  This matches current lightfunc behavior
    // but is not necessarily very intuitive.
    print(Object(pb) === pb);

    // ToString() coercion
    print(String(pb));

    // ToString goes through Uint8Array.prototype
    Uint8Array.prototype.toString = function () { return '[Overridden]'; };
    print(String(pb));
    delete Uint8Array.prototype.toString;

    // ToString(); overridden .toString() and .valueOf() which return a
    // plain buffer; causes a TypeError (matches V8 behavior for Uint8Array)
    Uint8Array.prototype.toString = function () { return createPlainBuffer('abcd'); };
    Uint8Array.prototype.valueOf = function () { return createPlainBuffer('abcd'); };
    try {
        print(String(pb));
    } catch (e) {
        print(e);
    }
    delete Uint8Array.prototype.toString;
    delete Uint8Array.prototype.valueOf;

    // ToNumber() coerces via ToString(); usually results in NaN but by
    // overriding .toString() one can get a number result
    print(Number(pb));
    try {
        Uint8Array.prototype.toString = function () { return '123'; };
        print(Number(pb));
    } catch (e) {
        print(e);
    }
    delete Uint8Array.prototype.toString;

    // Object coercion creates an actual Uint8Array object, which also
    // has a .buffer property pointing to an ArrayBuffer.  All of these
    // have the same backing storage.

    pb = createPlainBuffer('abcdefghijklmnop');
    print(typeof pb, Object.prototype.toString.call(pb), typeof pb.buffer);
    obj = Object(pb);
    print(obj == pb);
    print(typeof obj, Object.prototype.toString.call(obj), typeof obj.buffer);
    ab = obj.buffer;
    print(typeof ab, Object.prototype.toString.call(ab));
    print(Duktape.enc('jx', pb));
    print(Duktape.enc('jx', obj));
    pb[2] = 0x55;
    print(Duktape.enc('jx', pb));
    print(Duktape.enc('jx', obj));
    print(Duktape.enc('jx', new Uint8Array(ab)));
}

try {
    print('coercion test');
    coercionTest();
} catch (e) {
    print(e.stack || e);
}
