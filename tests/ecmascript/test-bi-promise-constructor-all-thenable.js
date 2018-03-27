/*---
{
    "skip": true
}
---*/

/*===
done
O1.then
O2.then
Promise.all then
2 123 234
===*/

var O1CB, O2CB;
var O1 = { then: function (cb) { print('O1.then'); O1CB = cb; } };
var O2 = { then: function (cb) { print('O2.then'); O2CB = cb; } };
var P = Promise.all([ O1, O2 ]);
P.then(function (v) {
    print('Promise.all then');
    print(v.length, v[0], v[1]);
});

Promise.resolve().then(function () {
    O1CB(123);
    O2CB(234);
});

print('done');
