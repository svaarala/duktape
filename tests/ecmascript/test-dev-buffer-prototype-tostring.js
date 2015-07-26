/*
 *  Duktape.Buffer.prototype String coercion.
 *
 *  Duktape 1.2: String(Duktape.Buffer.prototype) would result in the string
 *  "undefined".
 *
 *  Duktape 1.3: String(Duktape.Buffer.prototype) is a TypeError.
 */

/*---
{
    "custom": true
}
---*/

/*===
object
function
object
true
buffer
object
object
true
TypeError
===*/

function test() {
    var buf = Duktape.Buffer('dummy');
    var obj = new Duktape.Buffer(buf);

    print(typeof Duktape);
    print(typeof Duktape.Buffer);
    print(typeof Duktape.Buffer.prototype);
    print('prototype' in Duktape.Buffer);

    print(typeof buf);
    print(typeof obj);
    print(typeof Object.getPrototypeOf(obj));
    print(Object.getPrototypeOf(obj) === Duktape.Buffer.prototype);

    // Behavior changed after Duktape 1.2: string coercion is a TypeError
    try {
        var t = String(Duktape.Buffer.prototype);
        print(typeof t, JSON.stringify(t));
    } catch (e) {
        print(e.name);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
