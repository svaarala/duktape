/*---
{
    "custom": true
}
---*/

/*===
object constructor as function
pointer true
object true pointer
object true
object true object
===*/

print('object constructor as function');

function constructorAsFunctionTest() {
    var ptr_plain = Duktape.Pointer('dummy');
    var buf_plain = Duktape.dec('hex', 'deadbeef');
    var t;

    // Note: for plain pointer values and Pointer objects .valueOf()
    // returns the plain pointer.  This mimics how String.prototype.valueOf()
    // works.

    print(typeof ptr_plain, ptr_plain instanceof Duktape.Pointer);
    t = Object(ptr_plain);  // coerce plain pointer to full Pointer object
    print(typeof t, t instanceof Duktape.Pointer, typeof t.valueOf());

    // Plain buffers are treated like Uint8Arrays (wherever possible),
    // so that:
    //   - typeof plainBuffer == 'object'
    //   - plainBuffer.valueOf() returns Object-coerced version of buffer
    //     (a full Uint8Array object with same backing)

    print(typeof buf_plain, buf_plain instanceof Uint8Array);
    t = Object(buf_plain);  // coerce plain buffer to an Uint8Array object
    print(typeof t, t instanceof Uint8Array, typeof t.valueOf());
}

try {
    constructorAsFunctionTest();
} catch (e) {
    print(e.name);
}

/*===
object constructor as constructor
pointer true
object true pointer
object true
object true object
===*/

print('object constructor as constructor');

function constructorTest() {
    var ptr_plain = Duktape.Pointer('dummy');
    var buf_plain = Duktape.dec('hex', 'deadbeef');
    var t;

    print(typeof ptr_plain, ptr_plain instanceof Duktape.Pointer);
    t = new Object(ptr_plain);
    print(typeof t, t instanceof Duktape.Pointer, typeof t.valueOf());

    print(typeof buf_plain, buf_plain instanceof Uint8Array);
    t = new Object(buf_plain);
    print(typeof t, t instanceof Uint8Array, typeof t.valueOf());
}

try {
    constructorTest();
} catch (e) {
    print(e.name);
}
