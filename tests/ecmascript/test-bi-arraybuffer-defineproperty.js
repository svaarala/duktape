/*
 *  Duktape 3+ supports typed array index properties on Object.defineProperty().
 *  In Duktape 2.x and before a TypeError was triggered.
 */

/*@include util-buffer.js@*/

/*===
object 8 [object Uint8Array]
0
68
object 8 [object Uint8Array]
===*/

function test() {
    var buf = new ArrayBuffer(8);
    var u8 = new Uint8Array(buf);
    print(typeof u8, u8.length, Object.prototype.toString.call(u8));

    print(u8[4]);
    try {
        Object.defineProperty(u8, '4', {
            value: 68
        });
    } catch (e) {
        // Duktape 2.x and before: TypeError: property is virtual
        // Old Node.js: TypeError: Cannot redefine a property of an object with external array elements
        print(e.stack || e);
    }
    print(u8[4]);
    print(typeof u8, u8.length, Object.prototype.toString.call(u8));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
