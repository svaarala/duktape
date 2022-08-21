/*
 *  Specification bug in ES2016+, [[Set]] exotic for typed arrays applies even
 *  when typed array is not the direct receiver.  Test for fixed behavior, see
 *  https://github.com/tc39/ecma262/issues/1541.
 */

/*===
123.5
0
===*/

var u8 = new Uint8Array(10);
var child = Object.create(u8);
child[5] = 123.5;  // Does not affect 'u8'.
print(child[5]);
print(u8[5]);
