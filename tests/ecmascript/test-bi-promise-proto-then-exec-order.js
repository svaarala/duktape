/*---
{
    "skip": true
}
---*/

/*===
done
then fulfill 1: 123
then fulfill 2: 123
then fulfill 4: 123
then fulfill 5: 123
then fulfill 3: 123
then fulfill 6: 123
===*/

var resolveP, rejectP;
var P = new Promise(function (resolve, reject) {
    resolveP = resolve; rejectP = reject;
});

// Before settling.
P.then(function (v) {
    print('then fulfill 1:', v);
}, function (e) {
    print('then reject 1:', String(e));
});

// Before settling, inside 'then'.
P.then(function (v) {
    print('then fulfill 2:', v);
    P.then(function (v) {
        print('then fulfill 3:', v);
    }, function (e) {
        print('then reject 3:', String(e));
    });
}, function (e) {
    print('then reject 2:', String(e));
});

// Settle; this just queues existing reactions.
resolveP(123);

// After settling, queued immediately.
P.then(function (v) {
    print('then fulfill 4:', v);
}, function (e) {
    print('then reject 4:', String(e));
});

// After settling, inside 'then', queued immediately.
P.then(function (v) {
    print('then fulfill 5:', v);
    P.then(function (v) {
        print('then fulfill 6:', v);
    }, function (e) {
        print('then reject 6:', String(e));
    });
}, function (e) {
    print('then reject 5:', String(e));
});

print('done');
