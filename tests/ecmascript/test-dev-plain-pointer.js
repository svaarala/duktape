/*
 *  Some plain pointer behavior tests
 */

/*---
{
    "custom": true
}
---*/

/*===
basic test
pointer
object
[object Pointer]
[object Pointer]
true
true
===*/

function basicTest() {
    var plain = Duktape.Pointer('dummy');
    var object = new Duktape.Pointer('dummy');

    print(typeof plain);
    print(typeof object);

    print(Object.prototype.toString.call(plain));
    print(Object.prototype.toString.call(object));

    print(plain instanceof Duktape.Pointer);  // Changed in Duktape 2.x to be true
    print(object instanceof Duktape.Pointer);
}

try {
    print('basic test');
    basicTest();
} catch (e) {
    print(e.stack || e);
}
