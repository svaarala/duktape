/*---
{
    "skip": true
}
---*/

/*===
done
all fulfill: 123,234,345
all fulfill: 123,234,345
===*/

var resolve1, reject1;
var resolve2, reject2;
var resolve3, reject3;

var P1 = new Promise(function (resolve, reject) {
    resolve1 = resolve; reject1 = reject;
});
var P2 = new Promise(function (resolve, reject) {
    resolve2 = resolve; reject2 = reject;
});
var P3 = new Promise(function (resolve, reject) {
    resolve3 = resolve; reject3 = reject;
});

var P = Promise.all([ P1, P2, P3 ]);
P.then(function (v) {
    print('all fulfill:', v);
}, function (e) {
    print('all reject:', String(e));
});

resolve3(345);
resolve1(123);
resolve2(234);

// Another .then() after resolution.
P.then(function (v) {
    print('all fulfill:', v);
}, function (e) {
    print('all reject:', String(e));
});

print('done');
