/*---
{
    "custom": true
}
---*/

/*===
- the virtualized .prototype property appears in Object.getOwnPropertyNames()
fileName
length
prototype
- enumeration position
0 fileName
1 length
2 foo
3 prototype
[object Object]
0 fileName
1 length
2 foo
3 prototype
4 bar
- the virtualized .prototype property does not appear in for-in enumeration
before
[object Object]
after
done
===*/

print('- the virtualized .prototype property appears in Object.getOwnPropertyNames()');
var X = function () {};
Object.getOwnPropertyNames(X).forEach(function (v) {
    print(v);
});

// This behavior is liable to change without notice: the .prototype property
// does not have a stable order: its apparent position changes when the property
// is actually created.  This is not ideal and may be fixed later.
print('- enumeration position');
var X = function () {};
X.foo = 123;
Object.getOwnPropertyNames(X).forEach(function (v, i) {
    print(i, v);
});
print(X.prototype);  // creates property
X.bar = 321;
Object.getOwnPropertyNames(X).forEach(function (v, i) {
    print(i, v);
});

print('- the virtualized .prototype property does not appear in for-in enumeration');
print('before');
var X = function () {};
for (var k in X) {
    print(k);
}
print(X.prototype);  // creates property
print('after');
for (var k in X) {
    print(k);
}

print('done');
