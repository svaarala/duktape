/*===
7
300
100
500
700
200
400
600
===*/

var sym1 = Symbol();
var sym2 = Symbol();
var sym3 = Symbol();
var obj = {};
obj.foo = 100;
obj[sym1] = 200;
obj[3] = 300;
obj[sym2] = 400;
obj.quux = 500;
obj[sym3] = 600;
obj.baz = 700;

var keys = Reflect.ownKeys(obj);
print(keys.length);
keys.forEach(function (v) {
    print(obj[v]);
});
