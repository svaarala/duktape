/*---
{
    "skip": true
}
---*/

/*===
done
fulfill 1: 123
fulfill 2: undefined
fulfill 3: 321
reject 4: RangeError: aiee
fulfill 5: undefined
reject 6: RangeError: barf
reject 7: RangeError: woof
===*/

// Exercise onResolved/onFulfilled return value handling.

Promise.resolve(123).then(function (v) {
    print('fulfill 1:', v);
    // No return value <=> return void 0.
}, function (e) {
    print('reject 1:', e);
}).then(function (v) {
    print('fulfill 2:', v);
    // Explicit return value, fulfills.
    return 321;
}, function (e) {
    print('reject 2:', e);
}).then(function (v) {
    print('fulfill 3:', v);
    // Throw, rejects; reject is not caught by same level onRejected.
    throw new RangeError('aiee');
}, function (e) {
    print('reject 3:', e);
}).then(function (v) {
    print('fulfill 4:', v);
}, function (e) {
    print('reject 4:', String(e));
    // No return value <=> return void 0.
}).then(function (v) {
    print('fulfill 5:', v);
    throw new RangeError('barf');
}, function (e) {
    print('reject 5:', e);
}).then(function (v) {
    print('fulfill 6:', v);
}, function (e) {
    print('reject 6:', String(e));
    // Replacement error.
    throw new RangeError('woof');
}).then(function (v) {
    print('fulfill 7:', v);
}, function (e) {
    print('reject 7:', String(e));
});

print('done');
