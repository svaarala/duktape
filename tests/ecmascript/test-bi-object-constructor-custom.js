/*---
{
    "custom": true
}
---*/

/*===
object constructor as function
pointer false
object true pointer
buffer false
object true buffer
===*/

print('object constructor as function');

function constructorAsFunctionTest() {
    var ptr_plain = Duktape.Pointer('dummy');
    var buf_plain = Duktape.dec('hex', 'deadbeef');
    var t;

    print(typeof ptr_plain, ptr_plain instanceof Duktape.Pointer);
    t = Object(ptr_plain);
    print(typeof t, t instanceof Duktape.Pointer, typeof t.valueOf());

    print(typeof buf_plain, buf_plain instanceof Duktape.Buffer);
    t = Object(buf_plain);
    print(typeof t, t instanceof Duktape.Buffer, typeof t.valueOf());
}

try {
    constructorAsFunctionTest();
} catch (e) {
    print(e.name);
}

/*===
object constructor as constructor
pointer false
object true pointer
buffer false
object true buffer
===*/

print('object constructor as constructor');

function constructorTest() {
    var ptr_plain = Duktape.Pointer('dummy');
    var buf_plain = Duktape.dec('hex', 'deadbeef');
    var t;

    print(typeof ptr_plain, ptr_plain instanceof Duktape.Pointer);
    t = new Object(ptr_plain);
    print(typeof t, t instanceof Duktape.Pointer, typeof t.valueOf());

    print(typeof buf_plain, buf_plain instanceof Duktape.Buffer);
    t = new Object(buf_plain);
    print(typeof t, t instanceof Duktape.Buffer, typeof t.valueOf());
}

try {
    constructorTest();
} catch (e) {
    print(e.name);
}
