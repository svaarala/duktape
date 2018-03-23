/*---
{
    "skip": true
}
---*/

/*===
done
Q fulfill: 321
===*/

// Promise executor return value has no effect on promise value.

var resolveP, rejectP;
var resolveQ, rejectQ;
var P = new Promise(function (resolve, reject) {
    resolveP = resolve;
    rejectP = reject;
    return 123;  // return value won't resolve P
});
P.then(function (v) {
    print('P fulfill:', v);
}, function (e) {
    print('P reject:', e);
});
var Q = new Promise(function (resolve, reject) {
    resolveQ = resolve;
    rejectQ = reject;
    return 123;
});
Q.then(function (v) {
    print('Q fulfill:', v);
}, function (e) {
    print('Q reject:', e);
});

resolveQ(321);

print('done');
