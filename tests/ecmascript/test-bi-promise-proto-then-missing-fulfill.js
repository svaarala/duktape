/*---
{
    "skip": true
}
---*/

/*===
done
P2 fulfill: 123
P5 fulfill: 123
===*/

var resolveP1, rejectP1;
var P1 = new Promise(function (resolve, reject) {
    resolveP1 = resolve;
    rejectP1 = reject;
});

var P2 = P1.then(function (v) {
    print('P2 fulfill:', v);
    return v;
});

var P3 = P2.then(void 0, function (e) {
    print('P3 reject:', e);
    throw e;
});

var P4 = P3.then();

var P5 = P4.then(function (v) {
    print('P5 fulfill:', v);
}, function (e) {
    print('P5 reject:', e);
});

resolveP1(123);

print('done');
