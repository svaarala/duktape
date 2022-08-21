/*===
[1,2,3]
true
===*/

var P = new Proxy([1, 2, 3], {});
print(JSON.stringify(P));
print(Array.isArray(P));
