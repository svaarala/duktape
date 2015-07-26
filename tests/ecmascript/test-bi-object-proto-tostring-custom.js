/*---
{
    "custom": true
}
---*/

/*===
string [object String]
object [object String]
buffer [object Buffer]
object [object Buffer]
pointer [object Pointer]
object [object Pointer]
===*/

function test() {
    var plain_str = 'foo';  // string is for comparison
    var obj_str = new String(plain_str);
    var plain_buf = Duktape.dec('hex', '666f6f');
    var obj_buf = new Duktape.Buffer(plain_buf);
    var plain_ptr = Duktape.Pointer('foo');  // dummy ptr
    var obj_ptr = new Duktape.Pointer(plain_ptr);

    function f(v) {
        print(typeof v, Object.prototype.toString.call(v));
    }

    f(plain_str);
    f(obj_str);
    f(plain_buf);
    f(obj_buf);
    f(plain_ptr);
    f(obj_ptr);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
