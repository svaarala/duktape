/*
 *  String constructor behavior for custom types.
 */

/*---
{
    "custom": true
}
---*/

/*===
buffer
string 3 foo
object 3 foo
object
string 3 foo
object 3 foo
pointer
string
object
object
string
object
===*/

function test() {
    var buf, ptr, str;

    // Buffer values

    buf = Duktape.dec('hex', '666f6f');  // plain buffer value
    print(typeof buf);
    str = String(buf);
    print(typeof str, str.length, str);
    str = new String(buf);
    print(typeof str, str.length, str);

    buf = new Duktape.Buffer(Duktape.dec('hex', '666f6f'));  // full buffer object
    print(typeof buf);
    str = String(buf);
    print(typeof str, str.length, str);
    str = new String(buf);
    print(typeof str, str.length, str);

    // Pointer values: don't print the result because the result length,
    // contents and even format varies.

    ptr = Duktape.Pointer('dummy');  // plain pointer value
    print(typeof ptr);
    str = String(ptr);
    print(typeof str);
    str = new String(ptr);
    print(typeof str);

    ptr = new Duktape.Pointer('dummy');  // full pointer object
    print(typeof ptr);
    str = String(ptr);
    print(typeof str);
    str = new String(ptr);
    print(typeof str);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
