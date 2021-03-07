/*---
{
    "custom": true
}
---*/

/*===
- Object.getOwnPropertyDescriptor() spawns .prototype
object
object
true
false
false
done
===*/

print('- Object.getOwnPropertyDescriptor() spawns .prototype');
var X = function () {};
var pd = Object.getOwnPropertyDescriptor(X, 'prototype');
print(typeof pd);
print(typeof pd.value);
print(pd.writable);
print(pd.enumerable);
print(pd.configurable);

print('done');
