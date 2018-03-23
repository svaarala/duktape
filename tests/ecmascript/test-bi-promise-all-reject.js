/*---
{
    "skip": true
}
---*/

/*===
done
all reject: RangeError: aiee
all reject: RangeError: aiee
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

// Even if one of the inputs is never resolved, a single error causes the
// .all() Promise to be rejected.
resolve3(345);
//resolve1(123);
reject2(new RangeError('aiee'));

// Another .then() after resolution.
P.then(function (v) {
    print('all fulfill:', v);
}, function (e) {
    print('all reject:', String(e));
});

print('done');
