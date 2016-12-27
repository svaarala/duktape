/*===
[object Uint8Array]
[object Uint8Array]
number 10
number 3
===*/

function test() {
    var buf = new Buffer(10);
    var slice = buf.slice(1, 4);
    print(Object.prototype.toString.call(buf));
    print(Object.prototype.toString.call(slice));
    print(typeof buf.length, buf.length);
    print(typeof slice.length, slice.length);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
