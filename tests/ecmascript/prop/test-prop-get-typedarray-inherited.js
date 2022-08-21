/*===
123
===*/

var u8 = new Uint8Array(10);
var child = Object.create(u8);
u8[5] = 123;
print(child[5]);
