/*===
{"value":1,"writable":true,"enumerable":true,"configurable":true}
{"value":2,"writable":true,"enumerable":true,"configurable":true}
{"value":3,"writable":true,"enumerable":true,"configurable":true}
undefined
{"value":1,"writable":true,"enumerable":true,"configurable":true}
undefined
undefined
undefined
===*/

var u8 = new Uint8Array([ 1, 2, 3 ]);
var pd;

pd = Object.getOwnPropertyDescriptor(u8, 0);
print(JSON.stringify(pd));
pd = Object.getOwnPropertyDescriptor(u8, 1);
print(JSON.stringify(pd));
pd = Object.getOwnPropertyDescriptor(u8, 2);
print(JSON.stringify(pd));

pd = Object.getOwnPropertyDescriptor(u8, 3);
print(JSON.stringify(pd));

pd = Object.getOwnPropertyDescriptor(u8, '0');
print(JSON.stringify(pd));

pd = Object.getOwnPropertyDescriptor(u8, '+0');
print(JSON.stringify(pd));

pd = Object.getOwnPropertyDescriptor(u8, '-0');
print(JSON.stringify(pd));

pd = Object.getOwnPropertyDescriptor(u8, '0.0');
print(JSON.stringify(pd));
