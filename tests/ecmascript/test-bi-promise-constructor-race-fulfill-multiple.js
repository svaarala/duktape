/*---
{
    "skip": true
}
---*/

/*===
done
race fulfill: 234
race fulfill: 234
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

var P = Promise.race([ P1, P2, P3 ]);
P.then(function (v) {
    print('race fulfill:', v);
}, function (e) {
    print('race reject:', String(e));
});

// First resolve/reject wins.  ES2015 guarantees FIFO ordering for a certain
// job queue so that, here, it's guaranteed that the direct consequences of
// reject2 (settling Promise.race()) happen first.
resolve2(234);
reject1(new Error('aiee'));
resolve3(345);

// Another .then() after resolution.
P.then(function (v) {
    print('race fulfill:', v);
}, function (e) {
    print('race reject:', String(e));
});

print('done');
