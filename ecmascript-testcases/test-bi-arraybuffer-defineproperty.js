/*
 *  Duktape ArrayBuffer/view virtual properties don't work in
 *  Object.defineProperty().
 */

/*@include util-typedarray.js@*/

/*---
{
    "custom": true
}
---*/

/*===
object 8 [object Uint8Array]
0
TypeError
0
object 8 [object Uint8Array]
===*/

function test() {
    var buf = new ArrayBuffer(8);
    var u8 = new Uint8Array(buf);
    print(typeof u8, u8.length, String(u8));

    print(u8[4]);
    try {
        Object.defineProperty(u8, '4', {
            value: 68
        });
    } catch (e) {
        // Duktape: TypeError: property is virtual
        // Node.js: TypeError: Cannot redefine a property of an object with external array elements
        print(e.name);
        //print(e.stack || e);
    }
    print(u8[4]);
    print(typeof u8, u8.length, String(u8));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
