/*---
{
    "custom": true
}
---*/

print('- non-constructable built-ins have no .prototype');
var X = Math.cos;
print('prototype' in X);
print(X.prototype);
print('prototype' in X);

print('- non-constructable functions like object getter/setters have no .prototype');
var tmp = {
    get X() {},
};
X = Object.getOwnPropertyDescriptor(tmp, 'X').get;
print(typeof X);
print('prototype' in X);
print(X.prototype);
print('prototype' in X);

print('done');
