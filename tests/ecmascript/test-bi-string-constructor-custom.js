/*
 *  String constructor behavior for custom types.
 */

/*---
{
    "custom": true
}
---*/

/*===
object
string 19 [object Uint8Array]
object 19 [object Uint8Array]
object
string 20 [object ArrayBuffer]
object 20 [object ArrayBuffer]
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
    str = String(buf);  // since Duktape 2.x coerces (usually) to '[object Uint8Array]'
    print(typeof str, str.length, str);
    str = new String(buf);
    print(typeof str, str.length, str);

    buf = new Uint8Array([ 0x66, 0x6f, 0x6f ]).buffer;  // full ArrayBuffer object
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
