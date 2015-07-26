/*
 *  Differences in Node.js and Duktape Buffer behavior
 *
 *  This is not an exhaustive list of differences, just demonstrating
 *  some known issues.
 */

/*@include util-nodejs-buffer.js@*/

/*---
{
    "custom": true
}
---*/

/*===
fill()
aaaaaaaa
ABCDEFGH
ABCaaaaa
toString()
ABCDEFGH
ABC


slice() prototype
false
true
true
===*/

function nodejsDifferencesTest() {
    var buf, buf2;
    var proto;

    /*
     *  buf.fill() offset coercion differs.  Duktape behavior is to integer
     *  coerce and clamp both arguments, allowing crossed offsets.  This
     *  matches toString() clamping behavior.
     */

    print('fill()');

    // If start offset is negative, Node.js throws a RangeError.
    // Duktape treats it like 0 (clamped).

    buf = new Buffer('ABCDEFGH');
    try {
        buf.fill(0x61, -1);  // Node.js: RangeError, Duktape: treat like 0, fill entire buffer
    } catch (e) {
        print(e.name);
    }
    print(String(buf));

    // If start offset is beyond buffer length, Node.js clamps the offset.
    // Duktape does the same.  Result is a silent success with no changes
    // to the buffer.

    buf = new Buffer('ABCDEFGH');
    try {
        buf.fill(0x61, 9);  // Node.js: clamp, Duktape: clamp; no change to buffer
    } catch (e) {
        print(e.name);
    }
    print(String(buf));

    // If end offset is beyond buffer length, Node.js throws a RangeError.
    // Duktape clamps to the end of the buffer.

    buf = new Buffer('ABCDEFGH');
    try {
        buf.fill(0x61, 3, 9);  // Node.js: RangeError, Duktape: clamp, fill [3,8[
    } catch (e) {
        print(e.name);
    }
    print(String(buf));

    /*
     *  buf.toString() offset coercion differs from Node.js.  Duktape
     *  behavior is to integer coerce and clamp both arguments, allowing
     *  crossed offsets.
     */

    print('toString()');
    buf = new Buffer('ABCDEFGH');

    // If start offset is negative, Node.js always outputs an empty string.
    // Duktape will clamp the negative value to zero and interpret the end
    // offset.

    print(buf.toString(undefined, -1));      // Node.js: "", Duktape: "ABCDEFGH"
    print(buf.toString(undefined, -10, 3));  // Node.js: "", Duktape: "ABC"

    // If end offset is negative, Node.js will ignore it.  Duktape will
    // clamp it to zero (or start offset if higher).

    print(buf.toString(undefined, 0, -1));   // Node.js: "ABCDEFGH", Duktape: ""
    print(buf.toString(undefined, 3, -1));   // Node.js: "DEFGH", Duktape: ""

    /*
     *  buf.slice() result is a Node.js Buffer whose internal prototype is
     *  copied from the argument (which may not be Buffer.prototype).
     */

    print('slice() prototype');
    buf = new Buffer('ABCDEFGH');
    proto = { name: 'my_proto' };
    Object.setPrototypeOf(proto, Buffer.prototype);
    Object.setPrototypeOf(buf, proto);
    buf2 = buf.slice(2, 6);
    print(Object.getPrototypeOf(buf2) === Buffer.prototype);  // Node.js: true, Duktape: false
    print(Object.getPrototypeOf(buf2) === proto);             // Node.js: false, Duktape: true
    print(buf2 instanceof Buffer);                            // still true because my_proto inherits from Buffer.prototype
}

try {
    nodejsDifferencesTest();
} catch (e) {
    print(e.stack || e);
}
