/*@include util-buffer.js@*/

/*===
basic test
object
[object Uint8Array]
true
===*/

function basicTest() {
    var pb = createPlainBuffer('ABCD');

    // typeof
    print(typeof pb);

    // class name in Object.prototype.toString()
    print(Object.prototype.toString.call(pb));

    // instanceof
    print(pb instanceof Uint8Array);
}

try {
    print('basic test');
    basicTest();
} catch (e) {
    print(e.stack || e);
}
