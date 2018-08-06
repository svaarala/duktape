/*===
bar
undefined
true
false
123
true
undefined
true
===*/

var A = { foo: 'bar' };
var P1 = new Proxy(A, {});
var P2 = new Proxy(P1, {});
var P3 = new Proxy(P2, {});

print(P3.foo);
print(P3.bar);
print('foo' in P3);
print('bar' in P3);
P3.quux = 123;
print(A.quux);
print(delete P3.quux);
print(A.quux);
print(delete P3.noSuch);
